/********************************************************/
// CPU��Ҫ��STM32F103--RAM�ڴ治С��64K	Flash�ڴ治С��128K
// ����������STM32F103RDT6��VET6����ͨ��
// �༭���ڣ�20150909
// editor by СС��
// ���꣺shop182385147.taobao.com
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
	TIM3->SR = 0;                        //�����־λ  
	if(PCin(9))                          //һ�������1   A7
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

