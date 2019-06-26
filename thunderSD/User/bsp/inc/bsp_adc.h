/*
*********************************************************************************************************
*
*	模块名称 : ADC驱动模块
*	文件名称 : bsp_adc.c
*	版    本 : V1.0
*	说    明 : adc头文件
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2014-01-08   armfly   正式发布
*
*	Copyright (C), 2013-2014,    
*
*********************************************************************************************************
*/

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

void bsp_InitADC(void);
void StopADC(void);
void StartADC(void);
extern uint32_t DmaTctime ;
extern uint16_t ADCSampleTime;
extern uint16_t ADCSampleTimeSave;
extern uint8_t  Phaseindex ;
#endif

/*****************************    *********************************/
