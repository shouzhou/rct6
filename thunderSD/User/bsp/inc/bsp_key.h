#ifndef BSP_KEY_H
#define BSP_KEY_H	 
	 
 


#define WK_UP   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//��ȡ����2 
 

 
#define WKUP_PRES	3		//WK_UP  

void bsp_InitKey(void);//IO��ʼ��
uint8_t bsp_KeyScan(uint8_t mode);  	//����ɨ�躯��					    
#endif



