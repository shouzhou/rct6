#include "bsp.h"

void bsp_InitKey(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTC时钟

 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0
	
} 

uint8_t bsp_KeyScan(uint8_t mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY1==0||KEY2 ==0))
	{
		 
		key_up=0;
		if(KEY1==0)         return  KEY1_PRES; 
        else if(KEY2 ==0)   return  KEY2_PRES;
        
	}else if(KEY1==1 && KEY2 ==1)key_up=1; 	     
	return 0;// 无按键按下
}
