

#ifndef __APP_FUNC_H
#define __APP_FUNC_H

#include "stdint.h"

//0	启动手动模式	0x000A	1	启动手动模式			0
//0	启动循环模式	0x000A	1	启动循环模式			1
//4	启动自动浓度	0x000A	1	启动自动浓度			3
//4	停止自动浓度	0x000A	1	停止自动浓度			4
//4	启动自动体积	0x000A	1	启动自动体积			5
//4	停止自动体积	0x000A	1	停止自动体积			6
//3	停止手动模式	0x000A	1	停止手动模式			2
//5	停止循环模式	0x000A	1	停止循环模式			7
#define  STARTMANUAL   0
#define  STARTLOOP     1
#define  STOPMANUAL    2
#define  STARTAUTODEN  3
#define  STOPAUTODEN   4
#define  STARTAUTOV    5
#define  STOPAUTOV     6
#define  STOPLOOP      7


void FuncManual(uint16_t mode);
void FuncAutoV(uint16_t mode);
void FuncAutoNH3(uint16_t mode);
void FuncLoop(uint16_t mode);

extern uint16_t g_mannualtimerun ,g_loopruntimerun  ,g_loopstoptimerun ,g_nh3runtimerun,g_nh3stoptimerun;
extern uint16_t g_DelayFanOffCount;


#endif

 
