#ifndef _SYSVARS_H_
#define _SYSVARS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_NUM_POTS 4
#define MAX_THRESH_POINTS 4



/*
    sys_flags[0] = pot_0_installed
    sys_flags[1] = pot_1_installed
    sys_flags[2] = pot_2_installed
    sys_flags[3] = pot_3_installed
    
*/

/*
    interrupt_status/ctrl[0] = stomp
    interrupt_status/ctrl[1] = pot0_zone_left
    interrupt_status/ctrl[2] = pot1_zone_left
    interrupt_status/ctrl[3] = pot2_zone_left
    interrupt_status/ctrl[4] = pot3_zone_left
*/



#define POT0_LEFT_ZONE_INTR_BPOS 1
#define POT1_LEFT_ZONE_INTR_BPOS 2
#define POT2_LEFT_ZONE_INTR_BPOS 3
#define POT3_LEFT_ZONE_INTR_BPOS 4

typedef struct sysvars_t
{
    uint16_t    sys_flags;                      //  SW read-only
    uint16_t    sys_ctrl;                       //  SW read/write
    uint16_t    pot_values[MAX_NUM_POTS];       //  SW read-only
    uint16_t    num_thresh[MAX_THRESH_POINTS];
    uint16_t    pot_thresh_low[MAX_NUM_POTS][MAX_THRESH_POINTS];
    uint16_t    pot_thresh_high[MAX_NUM_POTS][MAX_THRESH_POINTS];
    uint16_t    interrupt_ctrl;                 //  SW read/write
    uint16_t    interrupt_status;               //  SW read-only
    uint16_t    fw_version;                     //  SW read-only
    uint8_t     i2c_addr;                       
    bool        interrupt_out;
    bool        clear_intr;
} sysvars_t;

extern volatile sysvars_t sysvars;

#ifdef __cplusplus
}
#endif

#endif //_SYSVARS_H_