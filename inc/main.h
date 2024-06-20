#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C"{
#endif

#define FIRMWARE_VERSION 0x0

#define I2C_SLAVE_BASE_ADDRESS  0x15    //  0b0 <00> 1 0101
#define I2C_SLAVE_BAUDRATE      100000  //  100kHz

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





#ifdef __cplusplus
}
#endif
#endif // _MAIN_H_