/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include "bsp.h"

extern  DHT11_T  g_DHT11;
void Peripheral_Init(void)
{
    uint8_t ret;
  	bsp_InitLed();											//LED接口初始化
    bsp_InitDHT11();
   
    ret = DHT11_ReadData(&g_DHT11);
    #ifdef USEDEBUG
    if (ret == 1)
    {
        printf("temp= %d  humi =  %d \r\n",g_DHT11.Temp, g_DHT11.Hum);
    }
    else
    {
        printf("\r\n未发现DHT22温湿度传感器\r\n");
    }
    #endif
    
    
    
    bsp_SendKey(1);
    bsp_DelayMS(1000);
    bsp_SendKey(6);
    bsp_DelayMS(1000);
    
    
	TIM3_CAP_Init(0xffff,72-1);
    
    BASIC_TIM_Init();
   // bsp_InitHardTimer();
    #ifdef USEDEBUG
    printf("CurMenuIndex = %d\r\n",CurMenuIndex);
    #endif
    bsp_SendKey(0x0A);  //如果屏幕处于别的界面 ，返回初始界面
    bsp_SendKey(0x06);
}
extern uint16_t sysruntime;
static bool _nbiot_init_state = false;
void nbiot_init_environment( int argc, char *argv[] )
{
    if ( !_nbiot_init_state )
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  

       // nbiot_time_init();
        bsp_InitTimer();
        bsp_InitRTC();
        bsp_InitUart();
        USART3_Init();
        bsp_InitIO(); 
        bsp_MkeyInit();        
        bsp_DelayMS(5000);
        bsp_InitI2C();
        #ifdef USEDEBUG
            if(ee_CheckOk()) printf("eeprom initial ok！\r\n");
            else  printf("eeprom initial error!\r\n");
        #endif
        //----------------- exec once ---------------------
        
      //  ee_WriteBytes(1,eeResult,0,EE_SIZE); //先全部清除
        //-------------------------------------------
        ee_ReadSaveData();
        bsp_Diwen_Updatedata(0x0008,0x01);

        bsp_Diwen_Updatedata(0x0003,g_DHT11.Temp*10);
        bsp_Diwen_Updatedata(0x0004,g_DHT11.Hum*10);
        bsp_Diwen_Updatedata(0x0005,300);
        bsp_Diwen_Updatedata(0x0006,sysruntime/6);

        Peripheral_Init(); 

        bsp_DelayMS(1000);			
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
//    开发板 RESET 接单片机 IO， 平时为低电平，如需复位单片机输出高电平 1S 再拉低。
//     开发板 PWR_EN 接单片机 IO， 平时为高电平， 如需给开发板整体断电请拉低。
    // m5310A复位  --reset输出高电平1s  再拉低
    bsp_IOOn(5);
    bsp_DelayMS(1000);
    bsp_IOOff(5);
    // 写失败次数数据进入EEPROM
     //1. 先读取数据
//     ee_ReadBytes(eeResult,6,4);
//     if(
    
    // 2. 再写入数据
    
    //stm32 复位
    __set_FAULTMASK(1);
    //NVIC_GenerateSystemReset(); //3.5.0
    NVIC_SystemReset(); // 3.6.1

}
