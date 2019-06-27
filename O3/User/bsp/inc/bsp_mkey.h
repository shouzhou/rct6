#ifndef BSP_MKEY_H
#define BSP_MKEY_H	 
	 
#define  KEYPAD_1  16
#define  KEYPAD_2  12
#define  KEYPAD_3  8
#define  KEYPAD_4  15
#define  KEYPAD_5  11
#define  KEYPAD_6  7
#define  KEYPAD_7  14
#define  KEYPAD_8  10
#define  KEYPAD_9  6
#define  KEYPAD_0  9
#define  KEYPAD_A  4
#define  KEYPAD_B  3
#define  KEYPAD_C  2
#define  KEYPAD_D  1
#define  KEYPAD_STAR  13
#define  KEYPAD_HASH  5


extern uint8_t keyvalue;
void bsp_MkeyInit(void);//IO初始化
uint8_t bsp_MkeyScan(void);   //按键扫描函数
uint8_t ReScan(GPIO_TypeDef* GPIOX,uint16_t colPin,uint8_t colIndex);//二次扫描
void bsp_KeyProcess(uint8_t key);
#endif
