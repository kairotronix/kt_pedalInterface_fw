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
        gpio_put(HEARTBEAT_PIN, 1)
}

repeating_timer_callback_t adc_read_callback()
{
    readAdcs = true;
}

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event)
{
    uint16_t buffer;
    uint8_t outBuffer;
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
                        case CTRL_ADDR_GENERAL_STATUS:
                            //  Read only
                            break;
                        case CTRL_ADDR_POT0:
                            //  Write to the threshold control
                            sysvars.pot_thresh_int[0] = buffer;
                            break;
                        case CTRL_ADDR_POT1:
                            sysvars.pot_thresh_int[1] = buffer;
                            break;
                        case CTRL_ADDR_POT2:
                            sysvars.pot_thresh_int[2] = buffer;
                            break;
                        case CTRL_ADDR_POT3:
                            sysvars.pot_thresh_int[3] = buffer;
                            break;
                        case CTRL_ADDR_INTRPT_CTRL:
                            sysvars.interrupt_ctrl = buffer;
                            break;
                        case CTRL_ADDR_INTRPT_STATUS:
                            //  Read only
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
                //  We are acknowleding an i2c transaction
                gpio_put(INTR_OUT_PIN, 0);
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
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_GENERAL_STATUS:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.sys_flags >> 8) : (0x00FF & sysvars.sys_flags);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_POT0:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[0] >> 8) : (0x00FF & sysvars.pot_values[0]);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_POT1:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[1] >> 8) : (0x00FF & sysvars.pot_values[1]);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_POT2:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[2] >> 8) : (0x00FF & sysvars.pot_values[2]);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_POT3:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.pot_values[3] >> 8) : (0x00FF & sysvars.pot_values[3]);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_INTRPT_CTRL:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.interrupt_ctrl >> 8) : (0x00FF & sysvars.interrupt_status);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                case CTRL_ADDR_INTRPT_STATUS:
                    outBuffer = (context.counter == 0) ? (uint8_t) (0x00FF & sysvars.interrupt_status >> 8) : (0x00FF & sysvars.interrupt_status);
                    if(context.counter == 0) context.counter++;
                    else context.counter = 0;
                    break;
                default :
                    outBuffer = 0;
                    break; 
            }
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

static void slave_setup()
{
    gpio_init(SLAVE_SDA_PIN);
    gpio_init(SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    //  May not need this on actual board due to presence of pull-up resistor...
    gpio_pull_up(SLAVE_SDA_PIN);
    
    gpio_init(SLAVE_SCL_PIN);
    gpio_set_function(SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_SLAVE_BAUDRATE);
    i2c_slave_init(i2c0, sysvars.i2c_addr, &i2c_slave_handler);
}

static void hw_init()
{
    sysvars.i2c_addr = I2C_SLAVE_BASE_ADDRESS;
    sysvars.fw_version = FIRMWARE_VERSION;
    sysvars.sys_ctrl = 0;
    sysvars.sys_flags = 0;
    sysvars.interrupt_ctrl = 0;
    sysvars.interrupt_status = 0;
    for(int i = 0; i < MAX_NUM_POTS; i++)
    {
        sysvars.pot_thresh_int[i] = 0;
        sysvars.pot_values[i] = 0;
    }
    gpio_set_dir(ADDR6_PIN, GPIO_IN);
    gpio_set_dir(ADDR5_PIN, GPIO_IN);
    gpio_set_dir(CONN0_PIN, GPIO_IN);
    gpio_set_dir(CONN1_PIN, GPIO_IN);
    gpio_set_dir(CONN2_PIN, GPIO_IN);
    gpio_set_dir(CONN3_PIN, GPIO_IN);
    gpio_set_dir(STOMP_PIN, GPIO_IN);
    gpio_set_dir(HEARTBEAT_PIN, GPIO_OUT);
    gpio_set_dir(INTR_OUT_PIN, GPIO_OUT);
    gpio_put(HEARTBEAT_PIN, 0);
    gpio_put(INTR_OUT_PIN, 0);
    gpio_pull_down(ADDR5_PIN);
    gpio_pull_down(ADDR6_PIN);
    sysvars.i2c_addr |= (uint8_t) (gpio_get(ADDR6_PIN) & 0x1) << 6;
    sysvars.i2c_addr |= (uint8_t) (gpio_get(ADDR5_PIN) & 0x1) << 5;
    adc_init();
    for(int i = 0; i < MAX_NUM_POTS; i++)
    {
        sysvars.sys_flags |= (((uint16_t)gpio_get(CONN0_PIN+i) & 0x1) << i);
        //  There's a pot plugged into the connector
        if((sysvars.sys_flags >> i) & 0x0001)
        {
            adc_gpio_init(ADC0_PIN+i);
        }
    }
}

int main()
{
    stdio_init_all();
    hw_init();
    slave_setup();

    struct repeating_timer adc_timer;
    struct repeating_timer heartbeat_timer;;
    add_repeating_timer_ms(ADC_READ_WAITTIME_MS, adc_read_callback, NULL, &adc_timer);
    add_repeating_timer_ms(HEARTBEAT_WAITTIME_MS, heartbeat_callback, NULL, &heartbeat_timer);
    while(1)
    {
        //  monitor
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
                    //  Check against threshold
                    //  Set interrupt if threshold
                }
            }
            //  Handshake
            readAdcs = false;
        }
    }
    //  Clean up
    cancel_repeating_timer(&heartbeat_timer);
    cancel_repeating_timer(&adc_timer);
    return 0;
}