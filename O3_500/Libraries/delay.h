#ifndef DELEY_H
#define DELEY_H
#include <stdint.h>
void mDelay(unsigned short ms);
void uDelay(uint32_t nus);
void nbiot_sleep( int milliseconds);
void delay_ms_sub(uint16_t sub_ms);
void bsp_InitHardTimer(void);


#endif
