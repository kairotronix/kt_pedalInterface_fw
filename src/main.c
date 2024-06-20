#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "inc/sysvars.h"
#include "inc/main.h"
#include "lib/pico_i2c_slave/i2c_slave/include/i2c_slave.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "hardware/uart.h"



static struct
{
    uint8_t counter;
    uint8_t address;
    bool address_written;
} context;

volatile bool readAdcs;

repeating_timer_callback_t heartbeat_callback()
{
    if(gpio_get(HEARTBEAT_PIN))
        gpio_put(HEARTBEAT_PIN, 0);
    else
        gpio_put(HEARTBEAT_PIN, 1);
}

repeating_timer_callback_t adc_read_callback()
{
    readAdcs = true;
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
    gpio_init(SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    //  May not need this on actual board due to presence of pull-up resistor...
    gpio_pull_up(SLAVE_SDA_PIN);
    
    gpio_init(SLAVE_SCL_PIN);
    gpio_set_function(SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL_PIN);
}

static void hw_init()
{
    //  Initialize the sysvars structure
    sysvars.i2c_addr = I2C_SLAVE_BASE_ADDRESS;
    sysvars.fw_version = FIRMWARE_VERSION;
    sysvars.sys_ctrl = 0;
    sysvars.sys_flags = 0;
    sysvars.interrupt_ctrl = 0;
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