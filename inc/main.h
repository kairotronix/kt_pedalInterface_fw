#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C"{
#endif

#define FIRMWARE_VERSION 0x0

#define I2C_SLAVE_BASE_ADDRESS  0x15    //  0b0 <00> 1 0101
#define I2C_SLAVE_BAUDRATE      100000  //  100kHz

#define SLAVE_SDA_PIN 8
#define SLAVE_SCL_PIN 9

#define ADDR6_PIN 25
#define ADDR5_PIN 24

#define STOMP_PIN 15
#define HEARTBEAT_PIN 23
#define UART_TX_PIN 13
#define UART_RX_PIN 14
#define CONN0_PIN 15
#define CONN1_PIN 16
#define CONN2_PIN 17
#define CONN3_PIN 18

#define INTR_OUT_PIN 2
#define GPIO_PIN 3
#define PWR_STATUS_PIN 4

#define ADC0_PIN 26
#define ADC1_PIN ADC0_PIN+1
#define ADC2_PIN ADC1_PIN+1
#define ADC3_PIN ADC2_PIN+1

#define ADC_READ_WAITTIME_MS 10 //  100 Hz
#define HEARTBEAT_WAITTIME_MS 500

/*
Address map:
0. General Control
1. ADC0
2. ADC1
3. ADC2
4. ADC3
5. Interrupt control
6. Interrupt status
*/


enum ctrl_addr_map_e
{
    CTRL_ADDR_GENERAL_CTRL = 0,
    CTRL_ADDR_GENERAL_STATUS,
    CTRL_ADDR_POT0,
    CTRL_ADDR_POT1,
    CTRL_ADDR_POT2,
    CTRL_ADDR_POT3,
    CTRL_ADDR_INTRPT_CTRL,
    CTRL_ADDR_INTRPT_STATUS,
    CTRL_ADDR_NUM_REGS
} ctrl_addr_map_e;


#ifdef __cplusplus
}
#endif
#endif // _MAIN_H_