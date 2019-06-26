#ifndef BSP_KEY_H
#define BSP_KEY_H	 
	 
 


#define KEY1   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键2 
#define KEY2   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)//读取按键2 
 

 
#define KEY1_PRES	1		//  
#define KEY2_PRES	2		// 

void bsp_InitKey(void);//IO初始化
uint8_t bsp_KeyScan(uint8_t mode);  	//按键扫描函数					    
#endif



