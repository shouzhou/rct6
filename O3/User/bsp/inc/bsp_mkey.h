#ifndef BSP_MKEY_H
#define BSP_MKEY_H	 
	 

void bsp_MkeyInit(void);//IO��ʼ��
uint8_t bsp_MkeyScan(void);   //����ɨ�躯��
uint8_t ReScan(GPIO_TypeDef* GPIOX,uint16_t colPin,uint8_t colIndex);//����ɨ��
#endif
