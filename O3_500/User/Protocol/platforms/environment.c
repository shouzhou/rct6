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
        printf("\r\nδ����DHT22��ʪ�ȴ�����\r\n");
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
        //��ʼ��RTC SYSTICK UART 
        bsp_InitTimer();
        bsp_InitRTC();
        bsp_InitUart();
        USART3_Init();
        //�л�����
        bsp_DelayMS(10000);
        bsp_SendKey(1);
        bsp_InitI2C();
        if(ee_CheckOk())
        {
            #ifdef USEDEBUG
             printf("eeprom initial ok��\r\n");
             
            #endif
        }
        else 
        {
            #ifdef USEDEBUG
            printf("eeprom initial error!\r\n");
            #endif
        }
        
        
        //----------------- exec once ---------------------
        
        //ee_WriteBytes(1,eeResult,0,EE_SIZE); //��ȫ�����
        //-------------------------------------------
         ee_ReadSaveData();
        if(g_WithoutOnenet == 0)
        {
            if(!SendCmd("AT\r\n", "OK", 1000,5,5))
            {
                g_WithoutOnenet = 1;
                eeResult[7] = 60;  //��λʧ�ܴ���
                ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
            }
        }
        if(g_WithoutOnenet ==0)
        {
        // ��λNBģ��
        bsp_IOOn(6);
        bsp_DelayMS(1000);
        bsp_IOOff(6);        
        bsp_DelayMS(5000);
        //����ʱ��
        

        }
       // ������Ļ����
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
        if(g_WithoutOnenet == 0)   //ʹ��onnet ������� �Ž��г�ʼ��  
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

      // SendCmd(AT_CLOSED, "OK",100);  /*�˳���AP*/
       _nbiot_init_state = false;
    }
}

void nbiot_reset(void)
{
//    ������ RESET �ӵ�Ƭ�� IO�� ƽʱΪ�͵�ƽ�����踴λ��Ƭ������ߵ�ƽ 1S �����͡�
//     ������ PWR_EN �ӵ�Ƭ�� IO�� ƽʱΪ�ߵ�ƽ�� ���������������ϵ������͡�
    // m5310A��λ  --reset����ߵ�ƽ1s  ������
    bsp_IOOn(6);
    bsp_DelayMS(1000);
    bsp_IOOff(6);
    // дʧ�ܴ������ݽ���EEPROM
     //1. �ȶ�ȡ����
//     ee_ReadBytes(eeResult,6,4);
//     if(
    
    // 2. ��д������
    
    //stm32 ��λ
    __set_FAULTMASK(1);
    //NVIC_GenerateSystemReset(); //3.5.0
    NVIC_SystemReset(); // 3.6.1

}
