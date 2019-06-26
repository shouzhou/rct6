#ifndef BSP_KEY_H
#define BSP_KEY_H	 
	 
 


#define WK_UP   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键2 
 

 
#define WKUP_PRES	3		//WK_UP  

void bsp_InitKey(void);//IO初始化
uint8_t bsp_KeyScan(uint8_t mode);  	//按键扫描函数					    
#endif



