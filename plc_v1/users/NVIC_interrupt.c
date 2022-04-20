/********************************************************/
// CPU��Ҫ��STM32F103--RAM�ڴ治С��64K	Flash�ڴ治С��128K
// ����������STM32F103RDT6��VET6����ͨ��
// �༭���ڣ�20150909
// editor by СС��
// ���꣺shop182385147.taobao.com
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
     NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;                       //����1   �жϵ���
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState; 
     NVIC_Init(&NVIC_InitStructure);
     
     NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;               //DMA�����ж�����
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);
	
     NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;                       //����3   �жϵ���
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState; 
     NVIC_Init(&NVIC_InitStructure);
	  
	   NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	                       //��ʱ��3�ж�
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure);
		 
		 NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	                       //��ʱ��4�ж�
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure);
		 
		 /* Enable the TIM5 gloabal Interrupt */
     NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;	                       //��ʱ��5   �жϵ�����Ҫ����PLC��ʱ������
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure);
		 								   
		 /* ʹ�ܶ�ʱ�� TIM8_CC �ж� */
     NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
     NVIC_Init(&NVIC_InitStructure);		
				
	   NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	                        //��ʱ��2�ж�
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure); 
		 
	   NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;	                    //�ⲿ�ж�10-15
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure); 
		 
		 NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;	                      //�ⲿ�ж�9-5
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
     NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
     NVIC_Init(&NVIC_InitStructure); 
}

