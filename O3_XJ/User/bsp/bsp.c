/*
*********************************************************************************************************
*
*	模块名称 : BSP模块(For STM32F1XX)
*	文件名称 : bsp.c
*	版    本 : V1.0
*	说    明 : 这是硬件底层驱动程序模块的主文件。主要提供 bsp_Init()函数供主程序调用。
?			   主程序的每个c文件可以在开
*			  头	添加 #include "bsp.h" 来包含所有的外设驱动模块。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2015-08-30  armfly  正式发布
*
*	Copyright (C), 2015-2016,    
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "modbus_slave.h"
/*
*********************************************************************************************************
*	函 数 名: bsp_Init
*	功能说明: 初始化硬件设备。只需要调用一次。该函数配置CPU寄存器和外设的寄存器并初始化一些全局变量。
*			 全局变量。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Init(void)
{
    uint8_t ret;
    bsp_InitTimer();
    bsp_InitRTC();
    bsp_InitUart();
    bsp_InitKey();
    //USART3_Init();
    bsp_DelayMS(3000);
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
    bsp_InitIO(); 
    bsp_InitLed();											//LED接口初始化
    bsp_InitDHT11();
    ret = DHT11_ReadData(&g_DHT11);
    #ifdef USEDEBUG
    if (ret == 1)
    {
        printf("temp= %d  humi =  %d \r\n",g_DHT11.Temp, g_DHT11.Hum);
        bsp_Diwen_Updatedata(0x0021,g_DHT11.Temp);
        bsp_DelayMS(5);
        bsp_Diwen_Updatedata(0x0021,g_DHT11.Temp);
        bsp_DelayMS(5);
        bsp_Diwen_Updatedata(0x0022,g_DHT11.Hum);
        bsp_DelayMS(5);
      //  bsp_Diwen_Updatedata(0x0006,g_sysruntime/6);
    }
    else
    {
        printf("\r\n未发现DHT22温湿度传感器\r\n");
    }
    #endif

     ADC1_GPIO_Configuration();
    ADC1_DMA_RCC_Configuration();
    ADC1_DMA_Init();
    //  bsp_DisableTouch();
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer10ms
*	功能说明: 该函数每隔10ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些处理时间要求不严格的
*			任务可以放在此函数。比如：按键扫描、蜂鸣器鸣叫控制等。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
//    static uint8_t cc;
//    cc++;
//    if(cc==100)
//    {
//        cc =0;
//       // bsp_LedToggle(1);
//    }
//    
    bsp_KeyScan();
        
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer1ms
*	功能说明: 该函数每隔1ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些需要周期性处理的事务
*			 可以放在此函数。比如：触摸坐标扫描。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: bsp_Idle
*	功能说明: 空闲时执行的函数。一般主程序在for和while循环程序体中需要插入 CPU_IDLE() 宏来调用本函数。
*			 本函数缺省为空操作。用户可以添加喂狗、设置CPU进入休眠模式的功能。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
extern void SaveScreenToBmp(uint16_t _index);
void bsp_Idle(void)
{
	/* --- 喂狗 */

	/* --- 让CPU进入休眠，由Systick定时中断唤醒或者其他中断唤醒 */

	/* 例如 emWin 图形库，可以插入图形库需要的轮询函数 */
	//GUI_Exec();

	/* 例如 uIP 协议，可以插入uip轮询函数 */

	
}

 
