/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*   系统 三个LED D4 D5 D6  
*   D4 每0.5秒 闪烁一次 表示系统正常运行
*   D5 modbus 每通讯一次 则闪烁一下 无通讯则处于不闪烁状态
*   D6 传感器每发送一次数据并接收到后 闪烁一次 无数据则不闪烁

*   更改modbus从机地址后 需要从新启动才能生效
*   
*********************************************************************************************************
*/

#include "bsp.h"			/* 底层硬件驱动 */
#include "modbus_slave.h"
#include "main.h"


PRINT_MODS_T g_tPrint;
extern VAR_T g_tVar;

extern uint8_t send[10];
extern uint32_t g_iRunTime;
void SoftReset(void);
int main(void)
{
	bsp_Init();							/* 硬件初始化 */
	SADDR485 = bsp_Read485Address(); 
    
            
    HC_AskForData();
	while(!g_hcRevOK) //等待探头数据
    {   
        HC_AskForData();
        HC_Analyze();
        bsp_DelayMS(100);
        bsp_LedToggle(1);
        bsp_LedToggle(3);
        bsp_LedToggle(2);
    }
	while (1)
	{
      
//        if(g_iRunTime % 70000 ==0)
//        {
//            __set_FAULTMASK(1);
//            NVIC_SystemReset(); 
//        }
		MODS_Poll();
		HC_Analyze();

        if(g_hcRevOK)
        {
            HC_AskForData();
            g_hcRevOK =0;
           // bsp_DelayMS(1000);
        }
	}
}







