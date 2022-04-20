
#ifndef _MAIN_H_
#define _MAIN_H_
#include "stm32f10x.h"
#include <plc_io.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "usart.h"

extern void PLC_ProInstructParse(void);
extern void RTC_Init(void);
extern void TIM3_PWM_Init(void);
extern void pulse_Init(void);
extern void RTC_Get(void);
extern void RST_C(void);
extern void RTC_Get_Init(void);
extern void PLC_Timer(void);
extern void A_B_Init(void);
extern void NCIC_Confinguration(FunctionalState NewState);
extern void ADC_init(void);
extern void red_init(void);
extern void backup_data(void);
extern void Recover_data(void);        //断电数据恢复  
extern void PLC_IO_config(void);
extern u8 Send_out;
extern void TX_Process(void);          //发送串口数据
extern void PLC_DATA_KEEP(void); 	

#endif

