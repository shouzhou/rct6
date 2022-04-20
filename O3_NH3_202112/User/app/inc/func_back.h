

#ifndef __APP_FUNC_H
#define __APP_FUNC_H

#include "stdint.h"

//0	�����ֶ�ģʽ	0x000A	1	�����ֶ�ģʽ			0
//0	����ѭ��ģʽ	0x000A	1	����ѭ��ģʽ			1
//4	�����Զ�Ũ��	0x000A	1	�����Զ�Ũ��			3
//4	ֹͣ�Զ�Ũ��	0x000A	1	ֹͣ�Զ�Ũ��			4
//4	�����Զ����	0x000A	1	�����Զ����			5
//4	ֹͣ�Զ����	0x000A	1	ֹͣ�Զ����			6
//3	ֹͣ�ֶ�ģʽ	0x000A	1	ֹͣ�ֶ�ģʽ			2
//5	ֹͣѭ��ģʽ	0x000A	1	ֹͣѭ��ģʽ			7
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

 
