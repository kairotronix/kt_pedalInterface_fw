#ifndef _SYSVARS_H_
#define _SYSVARS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_NUM_POTS 4

typedef struct sysvars_t
{
    uint16_t    sys_flags;                      //  SW read-only
    uint16_t    sys_ctrl;                       //  SW read/write
    uint16_t    pot_values[MAX_NUM_POTS];       //  SW read-only
    uint16_t    pot_thresh_int[MAX_NUM_POTS];   //  SW read-write
    uint16_t    interrupt_ctrl;                 //  SW read/write
    uint16_t    interrupt_status;               //  SW read-only
    uint16_t    i2c_addr;                       //  SW read-only
    uint16_t    fw_version;                     //  SW read-only
};


#ifdef __cplusplus
}
#endif

#endif //_SYSVARS_H_