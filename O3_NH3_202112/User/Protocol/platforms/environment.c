/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include "bsp.h"

extern  DHT11_T  g_DHT11;
void Peripheral_Init(void)
{
    uint8_t ret;
  	bsp_InitLed();											//LED�ӿڳ�ʼ��
    bsp_InitDHT11();
   
    ret = DHT11_ReadData(&g_DHT11);
    #ifdef USEDEBUG
    if (ret == 1)
    {
        printf("temp= %d  humi =  %d \r\n",g_DHT11.Temp, g_DHT11.Hum);
    }
    else
    {
        printf("\r\nδ����DHT22��ʪ�ȴ�����\r\n");
    }
    #endif
    
    
    
  //  bsp_SendKey(1);
    bsp_DelayMS(1000);
  //  bsp_SendKey(6);
   // bsp_DelayMS(1000);
    
    
	//TIM3_CAP_Init(0xffff,72-1);
    
  //  BASIC_TIM_Init();
   // bsp_InitHardTimer();
    
    ADC1_GPIO_Configuration();
	ADC1_DMA_RCC_Configuration();
	ADC1_DMA_Init();
    
    #ifdef USEDEBUG
    printf("CurMenuIndex = %d\r\n",CurMenuIndex);
    #endif
   // bsp_SendKey(0x0A);  //�����Ļ���ڱ�Ľ��� �����س�ʼ����
   // bsp_SendKey(0x06);
}
extern uint16_t sysruntime;
static bool _nbiot_init_state = false;
void nbiot_init_environment( int argc, char *argv[] )
{
   // if ( !_nbiot_init_state )
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  

       // nbiot_time_init();
        bsp_InitTimer();
        bsp_InitRTC();
        bsp_InitUart();
        USART3_Init();
        bsp_InitIO(); 
       // bsp_MkeyInit(); 
        bsp_ScreenChange(7);
        bsp_DelayMS(200);
        bsp_ScreenChange(7);
        bsp_DelayMS(200);        
       
        bsp_InitI2C();
        if(ee_CheckOk())
            #ifdef USEDEBUG
             printf("eeprom initial ok��\r\n");
             
            #endif
        else 
            #ifdef USEDEBUG
            printf("eeprom initial error!\r\n");
            #endif
        //----------------- exec once ---------------------
        
       // ee_WriteBytes(1,eeResult,0,EE_SIZE); //��ȫ�����
        //-------------------------------------------
        
        //bsp_Diwen_Updatedata(0x0008,0x01);

        bsp_Diwen_Updatedata(0x0004,g_DHT11.Temp);
        bsp_DelayMS(200);
        bsp_Diwen_Updatedata(0x0005,g_DHT11.Hum);
        bsp_DelayMS(200);
      //  bsp_Diwen_Updatedata(0x0005,300);
        bsp_Diwen_Updatedata(0x0003,0);
        bsp_DelayMS(200);
        Peripheral_Init(); 

        bsp_DelayMS(5000);
      //  bsp_DisableTouch();         //�رմ�������--0927
//        if(g_WithoutOnenet == 0)   //ʹ��onnet ������� �Ž��г�ʼ��     
//            netdev_init();
//        _nbiot_init_state = true;
    }
}




