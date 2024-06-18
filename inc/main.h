#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C"{
#endif

#define I2C_SLAVE_BASE_ADDRESS  0x15    //  0b0 <00> 1 0101
#define I2C_SLAVE_BAUDRATE      100000  //  100kHz

#define SLAVE_SDA_PIN 4
#define SLAVE_SCL_PIN 5

/*
CONTROL_WORD = `{write_nRead, addr[6:0]}`
Address map:
0. General Control
1. ADC0
2. ADC1
3. ADC2
4. ADC3
5. Interrupt control
6. Interrupt status
*/

#define CTRL_WORD_WRITENREAD_BPOS 7
#define CTRL_WORD_WRITENREAD_MASK (1<<CTRL_WORD_WRITENREAD_BPOS)
#define GET_CTRL_WRITENREAD(val) ((val & CTRL_WORD_WRITENREAD_MASK) >> CTRL_WORD_WRITENREAD_BPOS)


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