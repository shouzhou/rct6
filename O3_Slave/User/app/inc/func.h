

#ifndef __APP_FUNC_H
#define __APP_FUNC_H

#include "stdint.h"


void FunSwitch(void);
void FunFanOpen(void);
void FunFanClose(void);
void FunO3Control(uint8_t onnum,uint8_t index);
float FunO3CalParameter(void);

void bsp_SysCheck(void);
void FunTouch(void);


extern DHT11_T  g_DHT11;
#endif

 
