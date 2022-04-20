/********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include <stdio.h> 
#include "PLC_IO.h"
#include "PLC_Dialogue.h"

unsigned char Y0P,Y1P;

unsigned short Plus_CMP0,Plus_CMP1;


extern u16  PLC_16BIT[12100];


void TIM3_IRQHandler(void)
{   
	TIM3->SR = 0;                        //清除标志位  
	if(PCin(9))                          //一个脉冲加1   A7
	{ 
		PCout(9)=0;   
// 		PLC_RAM32(0X20005F1C)++;//D8140
		PLC_16BIT[0x078C]++;   //D8140
		
	}
	else
	PCout(9)=0;		
	
	if((PLC_16BIT[0xF9]&0x02)==0x02) 
	TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE),Y0P=0;		   
}

