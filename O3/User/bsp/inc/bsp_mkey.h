#ifndef BSP_MKEY_H
#define BSP_MKEY_H	 
	 

void bsp_MkeyInit(void);//IO初始化
uint8_t bsp_MkeyScan(void);   //按键扫描函数
uint8_t ReScan(GPIO_TypeDef* GPIOX,uint16_t colPin,uint8_t colIndex);//二次扫描
#endif
