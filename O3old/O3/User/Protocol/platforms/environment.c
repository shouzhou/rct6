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
        bsp_Diwen_Updatedata(0x0004,g_DHT11.Temp);
        bsp_DelayMS(5);
        bsp_Diwen_Updatedata(0x0004,g_DHT11.Temp);
        bsp_DelayMS(5);
        bsp_Diwen_Updatedata(0x0005,g_DHT11.Hum);
        bsp_DelayMS(5);
        bsp_Diwen_Updatedata(0x0006,g_sysruntime/6);
    }
    else
    {
        printf("\r\n未发现DHT22温湿度传感器\r\n");
    }
    #endif
    
    
    
    bsp_SendKey(1);
    bsp_DelayMS(100);
    bsp_SendKey(4);
    bsp_DelayMS(100);
    bsp_SendKey(5);
    bsp_DelayMS(100);
    
    
    
	TIM3_CAP_Init(0xffff,72-1);
    
   // BASIC_TIM_Init();
   // bsp_InitHardTimer();
    
    ADC1_GPIO_Configuration();
	ADC1_DMA_RCC_Configuration();
	ADC1_DMA_Init();
    bsp_DisableTouch();
}
extern uint16_t sysruntime;
static bool _nbiot_init_state = false;
void nbiot_init_environment( int argc, char *argv[] )
{
    if ( !_nbiot_init_state )
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  

       // nbiot_time_init();
         g_status  = STATUS_SYSINIBEGIN;
        //初始化RTC SYSTICK UART 
        bsp_InitTimer();
        bsp_InitRTC();
        bsp_InitUart();
        USART3_Init();
        //切换界面
        bsp_DelayMS(10000);
        bsp_SendKey(1);
        bsp_InitI2C();
        if(ee_CheckOk())
        {
            #ifdef USEDEBUG
             printf("eeprom initial ok！\r\n");
             
            #endif
        }
        else 
        {
            #ifdef USEDEBUG
            printf("eeprom initial error!\r\n");
            #endif
        }
        
        
        //----------------- exec once ---------------------
        
        //ee_WriteBytes(1,eeResult,0,EE_SIZE); //先全部清除
        //-------------------------------------------
         ee_ReadSaveData();
        if(g_WithoutOnenet == 0)
        {
            if(!SendCmd("AT\r\n", "OK", 1000,5,5))
            {
                g_WithoutOnenet = 1;
                eeResult[7] = 60;  //复位失败次数
                ee_WriteBytes(0,&eeResult[7],0x07,0x01); //写失败次数
            }
        }
        if(g_WithoutOnenet ==0)
        {
        // 复位NB模块
        bsp_IOOn(6);
        bsp_DelayMS(1000);
        bsp_IOOff(6);        
        bsp_DelayMS(5000);
        //更新时间
        

        }
       // 更新屏幕数据
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(100);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(100);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(100);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(100);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        
        bsp_InitIO(); 
        bsp_MkeyInit(); 
        
        
         Peripheral_Init(); 
        /////////////////////////SET TIME BEGIN//////////////////////
        if(g_WithoutOnenet ==0)
        {
        printf("start calibration time\r\n");
        SendCmd1("AT+CCLK?\r\n", "+CCLK", 2000,10,5);
        printf("stop calibration time\r\n");
        }
        /////////////////////////SET TIME end//////////////////////
        bsp_DelayMS(1000);
        if(g_WithoutOnenet == 0)   //使用onnet 的情况下 才进行初始化  
        {    
            g_status  = STATUS_NETINIBEGIN;  
                     
            netdev_init();
            g_status  = STATUS_NETINIEND;
            bsp_ScreenUpdateText();   
           
        }
        
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
    bsp_IOOn(6);
    bsp_DelayMS(1000);
    bsp_IOOff(6);
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
