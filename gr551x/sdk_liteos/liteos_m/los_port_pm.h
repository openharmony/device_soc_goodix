#ifndef __LOS_PORT_PM_H__
#define __LOS_PORT_PM_H__

#include <stdint.h>

void ultra_wfi(void);

uint32_t get_remain_sleep_dur(void);

void warm_boot_second(void);

#endif // __LOS_PORT_PM_H__