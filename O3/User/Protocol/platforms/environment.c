/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
//#include "platform.h"
//#include "M5310.h"
////#include "usart1.h"
////#include "usart3.h"
//#include "delay.h"

////#include "led.h"
//#include "beep.h"
//#include "BH1750.h"
//#include "i2c.h"
//#include "key.h"
//#include "adxl345.h"
//#include "BMP180.h"
//#include "SHT20.h"
#include "bsp.h"

void Peripheral_Init(void)
{
	
//	Led_Init();											//LED接口初始化
//	
//	Beep_Init();                                        //蜂鸣器初始化
//	
// 	Key_Init();											//按键接口初始化
//	
//	
//	IIC_Init();											//I2C总线初始化
//	
//	IIC_1750_Init();                                    //BH1750初始化
//	
//	ADXL345_Init();                                     //ADX345初始化
    bsp_Init();
	
}

static bool _nbiot_init_state = false;
void nbiot_init_environment( int argc, char *argv[] )
{
    if ( !_nbiot_init_state )
    {
     nbiot_time_init();
	// M5310_Power_Init();
    // USART1_Init();
        bsp_InitUart();
     USART3_Init(); 
     Peripheral_Init();
     printf("test\r\n");
#ifdef HARDWARE_TEST
     Usart2_Init(115200);	
	   Usart4_Init(115200);
    UsartPrintf(USART1, "I am USART1!\r\n");	
	  UsartPrintf(USART2, "I am USART2!\r\n");
	  UsartPrintf(USART3, "I am USART3!\r\n");
   	UsartPrintf(UART4,  "I am UART4!\r\n");
     Beep_test(500);	
		 Key_test();	
	   Led_test(500);
	   printf(".........SHT20 test begin.........\r\n");
	   SHT20_test();
		 Led1_Set(LED_OFF);
	   printf(".........SHT20 test end.........\r\n");
	   printf(".........BH1750 test begin.........\r\n");
	   BH1750_test();
		 Led2_Set(LED_OFF);
		 printf(".........BH1750 test end.........\r\n");
		 printf(".........ADXL345 test begin.........\r\n");
	   ADXL345_GetValue();
		 Led3_Set(LED_OFF);
		 printf(".........ADXL345 test end.........\r\n");
		 printf(".........BMP180 test begin.........\r\n");
	   BMP180_test();
		 Led4_Set(LED_OFF);
		 printf(".........BMP180 test end.........\r\n");
     mDelay(1000);
#endif			
     mDelay(5000);			
	   netdev_init();
     _nbiot_init_state = true;
    }
}

void nbiot_clear_environment( void )
{
    if ( _nbiot_init_state )
    {	

      // SendCmd(AT_CLOSED, "OK",100);  /*退出该AP*/
       _nbiot_init_state = false;
    }
}

void nbiot_reset(void)
{

 //NVIC_GenerateSystemReset();
    NVIC_SystemReset();

}



