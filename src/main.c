#include <stdint.h>
#include <stdio.h>
#include "pico/stdio.h"
#include <string.h>
#include <stdbool.h>


#include "../inc/sysvars.h"
#include "../inc/pinassign.h"
#include "../inc/main.h"

#include "hardware/regs/addressmap.h"
//#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
//#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "../lib/pico_i2c_slave/i2c_slave/include/i2c_slave.h"

//  We just need to store the system control register 
//  and the interrupt register into flash.. so 32-bits. 
//  We will use a whole block, however.

static struct
{
    uint8_t counter;
    uint8_t address;
    bool address_written;
} context;

volatile bool readAdcs;
volatile bool updateFlash;

#define PROGRAM_DATA_OFFSET (256*1024) // 256k from program memory

const uint8_t *flash_target_base = (const uint8_t *)(XIP_BASE + PROGRAM_DATA_OFFSET);



int write_config()
{
    
    uint8_t saveData[FLASH_PAGE_SIZE];
    uint8_t checkData[FLASH_PAGE_SIZE];
    saveData[0] = (uint8_t) ((sysvars.sys_ctrl >> 8) & 0x00ff);
    saveData[1] = (uint8_t) (sysvars.sys_ctrl & 0x00ff);
    saveData[2] = (uint8_t) ((sysvars.interrupt_ctrl >> 8) & 0x00ff);
    saveData[3] = (uint8_t) (sysvars.interrupt_ctrl & 0x00ff);
    for(int i = 4; i < FLASH_PAGE_SIZE; i++)
    {
        //  fill the rest with F's
        saveData[i] = 0xff;
    }
    //  Disable interrupts during write
    uint32_t interrupts = save_and_disable_interrupts();
    //  Erase one full sector after XIP
    flash_range_erase(PROGRAM_DATA_OFFSET, FLASH_SECTOR_SIZE);
    //  write the save data
    flash_range_program(PROGRAM_DATA_OFFSET,saveData,FLASH_PAGE_SIZE);
    
    //  restore interrupts
    restore_interrupts(interrupts);
    //  Check the data
    for(int i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        if(flash_target_base[i] != saveData[i])
            return -1;
    }
    //  Data all OK
    return 0;
}

void read_config()
{
    sysvars.sys_ctrl = ((( (uint16_t) flash_target_base[0] << 8) & 0xff00)) | ((( (uint16_t) flash_target_base[1]) & 0x00ff));
    sysvars.interrupt_ctrl = ((( (uint16_t) flash_target_base[2] << 8) & 0xff00)) | ((( (uint16_t) flash_target_base[3]) & 0x00ff));
}

void stomp_callback(uint gpio, uint32_t events)
{
    //  Always set the interrupt status just in case
    sysvars.interrupt_status |= (1<<STOMP_INTR_BPOS);
    //  If the interrupt is enabled, trigger an external interrupt
    if((sysvars.interrupt_ctrl >> STOMP_INTR_BPOS) & 0x1)
    {
        sysvars.interrupt_out = true;
    }
}


bool heartbeat_callback(repeating_timer_t *rt)
{
    if(gpio_get(HEARTBEAT_PIN))
        gpio_put(HEARTBEAT_PIN, 0);
    else
        gpio_put(HEARTBEAT_PIN, 1);
    return true;
}

bool adc_read_callback(repeating_timer_t *rt)
{
    readAdcs = true;
    return readAdcs;
}

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event)
{
    uint16_t buffer;
    uint8_t outBuffer;
    uint8_t temp;
    switch(event)
    {
        //  Master writes data
        case I2C_SLAVE_RECEIVE:
            //  Data from master
            if(context.address_written)
            {
                if(context.counter == 0)
                {
                    buffer = (uint16_t) i2c_read_byte_raw(i2c);
                    context.counter++;
                }
                else
                {
                    //  Read the last word in and reset counter
                    buffer <<= 8;
                    buffer &= 0xFF00;
                    buffer |= 0x00FF & (uint16_t) i2c_read_byte_raw(i2c);
                    switch(context.address)
                    {
                        case CTRL_ADDR_GENERAL_CTRL:
                            sysvars.sys_ctrl = buffer;
                            updateFlash = true;
                            break;
                        //  Begin read only
                        case CTRL_ADDR_GENERAL_STATUS:
                        case CTRL_ADDR_POT0:
                        case CTRL_ADDR_POT1:
                        case CTRL_ADDR_POT2:
                        case CTRL_ADDR_POT3:
                        case CTRL_ADDR_INTRPT_STATUS:
                        //  end read only
                            break;
                        case CTRL_ADDR_INTRPT_CTRL:
                            sysvars.interrupt_ctrl = buffer;
                            updateFlash = true;
                            break;
                        case CTRL_ADDR_NUM_THRESH_POT0:
                        case CTRL_ADDR_NUM_THRESH_POT1:
                        case CTRL_ADDR_NUM_THRESH_POT2:
                        case CTRL_ADDR_NUM_THRESH_POT3:
                            temp = context.address - CTRL_ADDR_NUM_THRESH_POT0;
                            //  maximum of MAX_THRESH_POINTS
                            sysvars.num_thresh[temp] = buffer <= MAX_THRESH_POINTS ? buffer : MAX_THRESH_POINTS;
                            break;
                        case CTRL_ADDR_THRESH_LOW0_POT0:
                        case CTRL_ADDR_THRESH_LOW0_POT1:
                        case CTRL_ADDR_THRESH_LOW0_POT2:
                        case CTRL_ADDR_THRESH_LOW0_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_LOW0_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_low[temp][0] = buffer;
                            //sysvars.thresh
                            break;
                        case CTRL_ADDR_THRESH_LOW1_POT0:
                        case CTRL_ADDR_THRESH_LOW1_POT1:
                        case CTRL_ADDR_THRESH_LOW1_POT2:
                        case CTRL_ADDR_THRESH_LOW1_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_LOW1_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_low[temp][1] = buffer;
                            break;
                        case CTRL_ADDR_THRESH_LOW2_POT0:
                        case CTRL_ADDR_THRESH_LOW2_POT1:
                        case CTRL_ADDR_THRESH_LOW2_POT2:
                        case CTRL_ADDR_THRESH_LOW2_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_LOW2_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_low[temp][2] = buffer;
                            break;
                        case CTRL_ADDR_THRESH_LOW3_POT0:
                        case CTRL_ADDR_THRESH_LOW3_POT1:
                        case CTRL_ADDR_THRESH_LOW3_POT2:
                        case CTRL_ADDR_THRESH_LOW3_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_LOW3_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_low[temp][3] = buffer;
                            break;
                        case CTRL_ADDR_THRESH_HIGH0_POT0:
                        case CTRL_ADDR_THRESH_HIGH0_POT1:
                        case CTRL_ADDR_THRESH_HIGH0_POT2:
                        case CTRL_ADDR_THRESH_HIGH0_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_HIGH0_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_high[temp][0] = buffer;
                            break;
                        case CTRL_ADDR_THRESH_HIGH1_POT0:
                        case CTRL_ADDR_THRESH_HIGH1_POT1:
                        case CTRL_ADDR_THRESH_HIGH1_POT2:
                        case CTRL_ADDR_THRESH_HIGH1_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_HIGH1_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_high[temp][1] = buffer;
                            break;
                        case CTRL_ADDR_THRESH_HIGH2_POT0:
                        case CTRL_ADDR_THRESH_HIGH2_POT1:
                        case CTRL_ADDR_THRESH_HIGH2_POT2:
                        case CTRL_ADDR_THRESH_HIGH2_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_HIGH2_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_high[temp][2] = buffer;
                            break;
                        case CTRL_ADDR_THRESH_HIGH3_POT0:
                        case CTRL_ADDR_THRESH_HIGH3_POT1:
                        case CTRL_ADDR_THRESH_HIGH3_POT2:
                        case CTRL_ADDR_THRESH_HIGH3_POT3:
                            temp = context.address - CTRL_ADDR_THRESH_HIGH3_POT0;
                            //  temp = which pot
                            //  threshold is second idx
                            sysvars.pot_thresh_high[temp][3] = buffer;
                            break;
                        default :
                            //  Ignore...
                            break; 
                    }
                    context.counter = 0;
                    context.address_written = false;
                }
            }
            else
            {
                //  We are going to read in the address
                context.address = i2c_read_byte_raw(i2c);
                context.address_written = true;
                //  force the counter to be 0 for continued logic...
                context.counter = 0;
            }
            break;
        case I2C_SLAVE_REQUEST:
            switch(context.address)
            {
                case CTRL_ADDR_GENERAL_CTRL:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.sys_ctrl >> 8) : (0x00FF & sysvars.sys_ctrl);
                    break;
                case CTRL_ADDR_GENERAL_STATUS:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.sys_flags >> 8) : (0x00FF & sysvars.sys_flags);
                    break;
                case CTRL_ADDR_POT0:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[0] >> 8) : (0x00FF & sysvars.pot_values[0]);
                    break;
                case CTRL_ADDR_POT1:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[1] >> 8) : (0x00FF & sysvars.pot_values[1]);
                    break;
                case CTRL_ADDR_POT2:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[2] >> 8) : (0x00FF & sysvars.pot_values[2]);
                    break;
                case CTRL_ADDR_POT3:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[3] >> 8) : (0x00FF & sysvars.pot_values[3]);
                    break;
                case CTRL_ADDR_INTRPT_CTRL:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.interrupt_ctrl >> 8) : (0x00FF & sysvars.interrupt_status);
                    break;
                case CTRL_ADDR_INTRPT_STATUS:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.interrupt_status >> 8) : (0x00FF & sysvars.interrupt_status);
                    //  We are acknowleding an i2c transaction
                    sysvars.interrupt_out = false;
                    sysvars.clear_intr = true;
                    break;
                case CTRL_ADDR_NUM_THRESH_POT0:
                case CTRL_ADDR_NUM_THRESH_POT1:
                case CTRL_ADDR_NUM_THRESH_POT2:
                case CTRL_ADDR_NUM_THRESH_POT3:
                    temp = context.address - CTRL_ADDR_NUM_THRESH_POT0;
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.num_thresh[temp] >> 8) : (0x00FF & sysvars.num_thresh[temp]);
                    break;
                case CTRL_ADDR_THRESH_LOW0_POT0:
                case CTRL_ADDR_THRESH_LOW0_POT1:
                case CTRL_ADDR_THRESH_LOW0_POT2:
                case CTRL_ADDR_THRESH_LOW0_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_LOW0_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_low[temp][0] >> 8) : (0x00FF & sysvars.pot_thresh_low[temp][0]);
                    //sysvars.thresh
                    break;
                case CTRL_ADDR_THRESH_LOW1_POT0:
                case CTRL_ADDR_THRESH_LOW1_POT1:
                case CTRL_ADDR_THRESH_LOW1_POT2:
                case CTRL_ADDR_THRESH_LOW1_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_LOW1_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_low[temp][1] >> 8) : (0x00FF & sysvars.pot_thresh_low[temp][1]);
                    break;
                case CTRL_ADDR_THRESH_LOW2_POT0:
                case CTRL_ADDR_THRESH_LOW2_POT1:
                case CTRL_ADDR_THRESH_LOW2_POT2:
                case CTRL_ADDR_THRESH_LOW2_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_LOW2_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_low[temp][2] >> 8) : (0x00FF & sysvars.pot_thresh_low[temp][2]);
                    break;
                case CTRL_ADDR_THRESH_LOW3_POT0:
                case CTRL_ADDR_THRESH_LOW3_POT1:
                case CTRL_ADDR_THRESH_LOW3_POT2:
                case CTRL_ADDR_THRESH_LOW3_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_LOW3_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_low[temp][3] >> 8) : (0x00FF & sysvars.pot_thresh_low[temp][3]);
                    break;
                case CTRL_ADDR_THRESH_HIGH0_POT0:
                case CTRL_ADDR_THRESH_HIGH0_POT1:
                case CTRL_ADDR_THRESH_HIGH0_POT2:
                case CTRL_ADDR_THRESH_HIGH0_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_HIGH0_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_high[temp][0] >> 8) : (0x00FF & sysvars.pot_thresh_high[temp][0]);
                    break;
                case CTRL_ADDR_THRESH_HIGH1_POT0:
                case CTRL_ADDR_THRESH_HIGH1_POT1:
                case CTRL_ADDR_THRESH_HIGH1_POT2:
                case CTRL_ADDR_THRESH_HIGH1_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_HIGH1_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_high[temp][1] >> 8) : (0x00FF & sysvars.pot_thresh_high[temp][1]);
                    break;
                case CTRL_ADDR_THRESH_HIGH2_POT0:
                case CTRL_ADDR_THRESH_HIGH2_POT1:
                case CTRL_ADDR_THRESH_HIGH2_POT2:
                case CTRL_ADDR_THRESH_HIGH2_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_HIGH2_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_high[temp][2] >> 8) : (0x00FF & sysvars.pot_thresh_high[temp][2]);
                    break;
                case CTRL_ADDR_THRESH_HIGH3_POT0:
                case CTRL_ADDR_THRESH_HIGH3_POT1:
                case CTRL_ADDR_THRESH_HIGH3_POT2:
                case CTRL_ADDR_THRESH_HIGH3_POT3:
                    temp = context.address - CTRL_ADDR_THRESH_HIGH3_POT0;
                    //  temp = which pot
                    //  threshold is second idx
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_thresh_high[temp][3] >> 8) : (0x00FF & sysvars.pot_thresh_high[temp][3]);
                default :
                    outBuffer = 0;
                    break; 
            }
            //  increment context counter
            if(context.counter == 0) context.counter++;
            else context.counter = 0;
            i2c_write_byte_raw(i2c, outBuffer);
            break;
        case I2C_SLAVE_FINISH:
            //  I think do nothing, but maybe I can set some kind of counter start that gets kicked 
            //  elsewhere to reset internal logic...
            break;
        default:
            break;
    }
}


static void init_gpio(void)
{
    gpio_set_dir(ADDR6_PIN, GPIO_IN);
    gpio_set_dir(ADDR5_PIN, GPIO_IN);
    gpio_set_dir(CONN0_PIN, GPIO_IN);
    gpio_set_dir(CONN1_PIN, GPIO_IN);
    gpio_set_dir(CONN2_PIN, GPIO_IN);
    gpio_set_dir(CONN3_PIN, GPIO_IN);
    gpio_set_dir(STOMP_PIN, GPIO_IN);
    gpio_set_dir(HEARTBEAT_PIN, GPIO_OUT);
    gpio_set_dir(INTR_OUT_PIN, GPIO_OUT);

    gpio_init(SLAVE_SDA_PIN);
    gpio_set_function(SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    //  May not need this on actual board due to presence of pull-up resistor...
    gpio_pull_up(SLAVE_SDA_PIN);
    
    gpio_init(SLAVE_SCL_PIN);
    gpio_set_function(SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL_PIN);

    gpio_set_irq_enabled_with_callback(STOMP_PIN, GPIO_IRQ_EDGE_RISE, true, &stomp_callback);
}

static void hw_init()
{
    //  Do flash things
    read_config();
    updateFlash=false;
    //  Initialize the sysvars structure
    sysvars.i2c_addr = I2C_SLAVE_BASE_ADDRESS;
    sysvars.fw_version = FIRMWARE_VERSION;
    sysvars.sys_flags = 0;
    sysvars.interrupt_status = 0;
    sysvars.interrupt_out = false;
    sysvars.clear_intr = false;
    for(int i = 0; i < MAX_NUM_POTS; i++)
    {
        sysvars.num_thresh[i] = 0;
        sysvars.pot_values[i] = 0;
        for(int j = 0; j < MAX_THRESH_POINTS; j++)
        {
            sysvars.pot_thresh_high[j][i] = 0;
            sysvars.pot_thresh_low[j][i] = 0;
        }
    }
    //  Initialize the GPIO
    init_gpio();
    //  Set heartbeat low initially
    gpio_put(HEARTBEAT_PIN, 0);
    //  Disable interrupt output initially
    gpio_put(INTR_OUT_PIN, 0);
    gpio_pull_down(ADDR5_PIN);
    gpio_pull_down(ADDR6_PIN);
    //  Get the i2c address from the switch bank
    sysvars.i2c_addr |= (uint8_t) (gpio_get(ADDR6_PIN) & 0x1) << 6;
    sysvars.i2c_addr |= (uint8_t) (gpio_get(ADDR5_PIN) & 0x1) << 5;
    //  Initialize the ADCs
    adc_init();
    //  Check if there are pots plugged into the ADCs
    for(int i = 0; i < MAX_NUM_POTS; i++)
    {
        sysvars.sys_flags |= (((uint16_t)gpio_get(CONN0_PIN+i) & 0x1) << i);
        //  There's a pot plugged into the connector
        if((sysvars.sys_flags >> i) & 0x0001)
        {
            adc_gpio_init(ADC0_PIN+i);
        }
    }

    //  Initialize the I2C slave driver
    i2c_init(i2c0, I2C_SLAVE_BAUDRATE);
    i2c_slave_init(i2c0, sysvars.i2c_addr, &i2c_slave_handler);
}

int main()
{
    struct repeating_timer adc_timer;
    struct repeating_timer heartbeat_timer;
    bool inZone[MAX_NUM_POTS][MAX_THRESH_POINTS];
    bool inZone_reg[MAX_NUM_POTS][MAX_THRESH_POINTS];
    bool leftZone[MAX_NUM_POTS][MAX_THRESH_POINTS];
    //  Initialize local variables
    for(int i = 0; i < MAX_NUM_POTS; i++)
    {
        for(int j = 0; j < MAX_THRESH_POINTS; j++)
        {
            inZone[i][j] = false;
            inZone_reg[i][j] = false;
            leftZone[i][j] = false;
        }
    }
    stdio_init_all();
    hw_init();

    //  FETCH SYSVARS FROM MEMORY

    //  Set up the ADC read timer
    add_repeating_timer_ms(ADC_READ_WAITTIME_MS, adc_read_callback, NULL, &adc_timer);
    //  Set up the heartbeat interrupt
    add_repeating_timer_ms(HEARTBEAT_WAITTIME_MS, heartbeat_callback, NULL, &heartbeat_timer);


    while(1)
    {
        if(updateFlash)
        {
            if(write_config())
            {
                //  log some kind of error for mismatch... for now do nothing
            }
            //  release the flag
            updateFlash = false;
        }
        //  Whenever the read timer passes, read all ADCs
        if(readAdcs)
        {
            //  read ADCs
            for(int i = 0; i < MAX_NUM_POTS; i++)
            {
                //  Check if there was something plugged in
                sysvars.sys_flags |= (((uint16_t)gpio_get(CONN0_PIN+i) & 0x1) << i);
                //  Select the input and read the value if valid
                if((sysvars.sys_flags >> i) & 0x0001)
                {
                    adc_select_input(ADC0_PIN+i);
                    sysvars.pot_values[i] = adc_read();
                    //  Check against thresholds
                    for(int j = 0; j < sysvars.num_thresh[i]; j++)
                    {
                        //  Figure out if we are within this zone
                        if((sysvars.pot_values[i] >= sysvars.pot_thresh_low[i][j]) && (sysvars.pot_values[i] < sysvars.pot_thresh_high[i][j]))
                        {
                            inZone[i][j] = true;
                        }
                        else
                        {
                            inZone[i][j] = false;
                        }
                        
                        //  If we left the zone, flag it
                        if(inZone_reg[i][j] && !inZone[i][j])
                        {
                            leftZone[i][j] = true;
                        }
                        inZone_reg[i][j] = inZone[i][j];
                        //  update interrupt status
                        if(leftZone[i][j])
                        {
                            sysvars.interrupt_status |= (1 << (POT0_LEFT_ZONE_INTR_BPOS + i));
                        }
                        //  set interrupt if the control is set
                        if(!sysvars.interrupt_out && leftZone[i][j] && (sysvars.interrupt_ctrl >> (POT0_LEFT_ZONE_INTR_BPOS+i) & 0x01))
                        {
                            sysvars.interrupt_out = true;
                            leftZone[i][j] = false;
                        }
                    }


                }
            }
            //  Handshake to release the flag
            readAdcs = false;
        }

        
        if(sysvars.clear_intr)
        {
            sysvars.clear_intr = false;
            sysvars.interrupt_status = 0;
            sysvars.interrupt_out = false;
            for(int i = 0; i < MAX_NUM_POTS; i++)
            {
                for(int j = 0; j < MAX_THRESH_POINTS; j++)
                {
                    leftZone[i][j] = false;
                }
            }
        }
        //  set interrupt pin
        if(sysvars.interrupt_out)
        {
            gpio_put(INTR_OUT_PIN, 1);
        }
        else
        {
            gpio_put(INTR_OUT_PIN, 0);
        }
    }
    //  Clean up
    cancel_repeating_timer(&heartbeat_timer);
    cancel_repeating_timer(&adc_timer);
    return 0;
}