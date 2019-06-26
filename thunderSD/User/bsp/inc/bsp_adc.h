/*
*********************************************************************************************************
*
*	ģ������ : ADC����ģ��
*	�ļ����� : bsp_adc.c
*	��    �� : V1.0
*	˵    �� : adcͷ�ļ�
*	�޸ļ�¼ :
*		�汾��    ����        ����     ˵��
*		V1.0    2014-01-08   armfly   ��ʽ����
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
