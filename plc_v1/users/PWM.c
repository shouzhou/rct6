#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include <stdio.h> 
#include <absacc.h> 
extern u16 all_data[];

void TIM3_GPIO_Config(void)                                         
{                                                                   
  GPIO_InitTypeDef GPIO_Initstructure;                              
	/* ---------------------------------------------------------------
  使能TIM3时钟                                                      
	PCLK1经过2倍频后作为TIM3的时钟源等于72MHz                                 
  --------------------------------------------------------------- */        
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);                      
                                                                            
  /* GPIOA和GPIOB 时钟启动*/                                                
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	                                                                          
	GPIO_Initstructure.GPIO_Pin=GPIO_Pin_6|GPIO_Pin_7;            
	GPIO_Initstructure.GPIO_Mode=GPIO_Mode_AF_PP;     //推挽式输出
	GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;               
  GPIO_Init(GPIOA,&GPIO_Initstructure);                         
                                                                
  GPIO_Initstructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;            
  GPIO_Init(GPIOB,&GPIO_Initstructure);	                        
}                                                                

void TIM3_Mode_Config(void)
{ 
  /***************************************************************	
	 TIM3配置：产生4路PWM信号与4个不同的工作周期：
   TIM3时钟 = 72兆赫，分频器= 0x0，TIM3计数器时钟= 72兆赫
   TIM3ARR寄存器= 999 = > TIM3频率= TIM3计数器时钟/（ARR + 1）
   TIM3频率= 72 kHz。
	****************************************************************/
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitstructure;
	
 // 自动重装载寄存器周期的值(定时时间)累计 0xFFFF个频率后产生个更新或者中断(也是说定时时间到)。 
  TIM_TimeBaseStructure.TIM_Period =65535;
  //时钟预分频数  例如 ：时钟频率=72/(时钟预分频+1)
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  // 使用的采样频率之间的分频比例
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  //定时器模式向上计数。  
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
  //初始化定时器3
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	/**************配置为PWM比较输出模式****************************/
	TIM_OCInitstructure.TIM_OCMode = TIM_OCMode_Toggle;	 
	
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitstructure.TIM_OutputState=TIM_OutputState_Enable;
	/*当定时器计数器的值小于CCR1_Va1是为高电频*/           
	TIM_OCInitstructure.TIM_OCPolarity=TIM_OCPolarity_High;
	/*设置通道1跳变值，当计数器计数到这个值时，电平发生跳变*/
	TIM_OCInitstructure.TIM_Pulse= 4000;
  /*使用通道1*/
	TIM_OC1Init(TIM3,&TIM_OCInitstructure);
	
	/* PWM1 Mode configuration: Channel2 */
	/*设置通道2跳变值，当计数器计数到这个值时，电平发生跳变*/
	TIM_OCInitstructure.TIM_Pulse=100;  
	/*使用通道3*/
	TIM_OC2Init(TIM3,&TIM_OCInitstructure);

	/* PWM1 Mode configuration: Channel3 */
	TIM_OCInitstructure.TIM_Pulse=1000;   
	/*使用通道3*/
	TIM_OC3Init(TIM3,&TIM_OCInitstructure);
	
  /* PWM1 Mode configuration: Channel4 */
	/*设置通道2跳变值，当计数器计数到这个值时，电平发生跳变*/
	TIM_OCInitstructure.TIM_Pulse=500;   
	/*使用通道4*/
	TIM_OC4Init(TIM3,&TIM_OCInitstructure);
	
	/*禁止预载寄存器*/
  TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Disable);
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Disable);
	
	TIM_ITConfig(TIM3,TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);
	
  TIM_Cmd(TIM3, ENABLE);                   //使能定时器3 
}	

void TIM3_IRQHandler(void)//1ms产生一次中断信号
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
