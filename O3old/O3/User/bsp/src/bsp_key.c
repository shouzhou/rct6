#include "bsp.h"

void bsp_InitKey(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//ʹ��PORTA,PORTCʱ��

 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0
	
} 

uint8_t bsp_KeyScan(uint8_t mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY1==0||KEY2 ==0))
	{
		 
		key_up=0;
		if(KEY1==0)         return  KEY1_PRES; 
        else if(KEY2 ==0)   return  KEY2_PRES;
        
	}else if(KEY1==1 && KEY2 ==1)key_up=1; 	     
	return 0;// �ް�������
}
