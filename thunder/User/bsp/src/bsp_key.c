#include "bsp.h"

void bsp_InitKey(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//ʹ��PORTA,PORTCʱ��

 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0
	
} 

uint8_t bsp_KeyScan(uint8_t mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(WK_UP==0))
	{
		 
		key_up=0;
		if(WK_UP==0)return WKUP_PRES; 
	}else if(WK_UP==1)key_up=1; 	     
	return 0;// �ް�������
}