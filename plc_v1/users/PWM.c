#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include <stdio.h> 
#include <absacc.h> 
extern u16 all_data[];

void TIM3_GPIO_Config(void)                                         
{                                                                   
  GPIO_InitTypeDef GPIO_Initstructure;                              
	/* ---------------------------------------------------------------
  ʹ��TIM3ʱ��                                                      
	PCLK1����2��Ƶ����ΪTIM3��ʱ��Դ����72MHz                                 
  --------------------------------------------------------------- */        
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);                      
                                                                            
  /* GPIOA��GPIOB ʱ������*/                                                
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	                                                                          
	GPIO_Initstructure.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;            
	GPIO_Initstructure.GPIO_Mode=GPIO_Mode_AF_PP;     //����ʽ���
	GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;               
  GPIO_Init(GPIOA,&GPIO_Initstructure);                         
                                                                
  GPIO_Initstructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;            
  GPIO_Init(GPIOB,&GPIO_Initstructure);	                        
}                                                                

void TIM3_Mode_Config(void)
{ 
  /***************************************************************	
	 TIM3���ã�����4·PWM�ź���4����ͬ�Ĺ������ڣ�
   TIM3ʱ�� = 72�׺գ���Ƶ��= 0x0��TIM3������ʱ��= 72�׺�
   TIM3ARR�Ĵ���= 999 = > TIM3Ƶ��= TIM3������ʱ��/��ARR + 1��
   TIM3Ƶ��= 72 kHz��
	****************************************************************/
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitstructure;
	
 // �Զ���װ�ؼĴ������ڵ�ֵ(��ʱʱ��)�ۼ� 0xFFFF��Ƶ�ʺ���������»����ж�(Ҳ��˵��ʱʱ�䵽)�� 
  TIM_TimeBaseStructure.TIM_Period =65535;
  //ʱ��Ԥ��Ƶ��  ���� ��ʱ��Ƶ��=72/(ʱ��Ԥ��Ƶ+1)
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  // ʹ�õĲ���Ƶ��֮��ķ�Ƶ����
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  //��ʱ��ģʽ���ϼ�����  
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
  //��ʼ����ʱ��3
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	/**************����ΪPWM�Ƚ����ģʽ****************************/
	TIM_OCInitstructure.TIM_OCMode = TIM_OCMode_Toggle;	 
	
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitstructure.TIM_OutputState=TIM_OutputState_Enable;
	/*����ʱ����������ֵС��CCR1_Va1��Ϊ�ߵ�Ƶ*/           
	TIM_OCInitstructure.TIM_OCPolarity=TIM_OCPolarity_High;
	/*����ͨ��1����ֵ�������������������ֵʱ����ƽ��������*/
	TIM_OCInitstructure.TIM_Pulse= 4000;
  /*ʹ��ͨ��1*/
	TIM_OC1Init(TIM3,&TIM_OCInitstructure);
	
	/* PWM1 Mode configuration: Channel2 */
	/*����ͨ��2����ֵ�������������������ֵʱ����ƽ��������*/
	TIM_OCInitstructure.TIM_Pulse=100;  
	/*ʹ��ͨ��3*/
	TIM_OC2Init(TIM3,&TIM_OCInitstructure);

	/* PWM1 Mode configuration: Channel3 */
	TIM_OCInitstructure.TIM_Pulse=1000;   
	/*ʹ��ͨ��3*/
	TIM_OC3Init(TIM3,&TIM_OCInitstructure);
	
  /* PWM1 Mode configuration: Channel4 */
	/*����ͨ��2����ֵ�������������������ֵʱ����ƽ��������*/
	TIM_OCInitstructure.TIM_Pulse=500;   
	/*ʹ��ͨ��4*/
	TIM_OC4Init(TIM3,&TIM_OCInitstructure);
	
	/*��ֹԤ�ؼĴ���*/
  TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Disable);
	
	TIM_ITConfig(TIM3,TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);
	
  TIM_Cmd(TIM3, ENABLE);                   //ʹ�ܶ�ʱ��3 
}	

void TIM3_IRQHandler(void)//1ms����һ���ж��ź�
{ 
	u16 Capture;
  if(TIM_GetITStatus(TIM3, TIM_IT_CC1) == SET)
  {	
    TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
		Capture=TIM_GetCapture1(TIM3);
		TIM_SetCompare1(TIM3,Capture+4000);
  }
	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) == SET)
  {	
    TIM_ClearITPendingBit(TIM3,TIM_IT_CC2);
		Capture=TIM_GetCapture2(TIM3);
		TIM_SetCompare2(TIM3,Capture+100);
  }
}

void TIM3_PWM_Init(void)
{
	TIM3_GPIO_Config();
	TIM3_Mode_Config();	
}
