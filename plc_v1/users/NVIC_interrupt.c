/********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "main.h"
#include <stdio.h>
#include <absacc.h> 

void NCIC_Confinguration(FunctionalState NewState)
{  
  	 NVIC_InitTypeDef NVIC_InitStructure;
		
  /* Set the Vector Table base address at 0x08000000 */
   	 NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3 );
  
  /* Enable the USART1 Interrupt */ 
     NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;                       //串口1   中断调用
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState; 
     NVIC_Init(&NVIC_InitStructure);
     
     NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;               //DMA发送中断设置
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);
	
     NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;                       //串口3   中断调用
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState; 
     NVIC_Init(&NVIC_InitStructure);
	  
	   NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	                       //定时器3中断
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure);
		 
		 NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	                       //定时器4中断
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure);
		 
		 /* Enable the TIM5 gloabal Interrupt */
     NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;	                       //定时器5   中断调用主要负责PLC定时器运行
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure);
		 								   
		 /* 使能定时器 TIM8_CC 中断 */
     NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
     NVIC_Init(&NVIC_InitStructure);		
				
	   NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	                        //定时器2中断
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure); 
		 
	   NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	                    //外部中断10-15
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure); 
		 
		 NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;	                      //外部中断9-5
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure); 
}

