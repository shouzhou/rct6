/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*   系统 三个LED D4 D5 D6  
*   D4 每0.5秒 闪烁一次 表示系统正常运行
*   D5 modbus 每通讯一次 则闪烁一下 无通讯则处于不闪烁状态
*   D6 传感器每发送一次数据并接收到后 闪烁一次 无数据则不闪烁(现在是2s通讯一次)

*   更改modbus从机地址后 需要从新启动才能生效
*   
*********************************************************************************************************
*/

#include "bsp.h"			/* 底层硬件驱动 */
#include "modbus_slave.h"
#include "main.h"

uint16_t g_RstCount=0;
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
	while(!g_hcDataOk) //等待探头数据
   // while(!g_hcRevOK) 
    {   
       
        HC_Analyze();
       // bsp_DelayMS(100);
         HC_AskForData();
      //  bsp_DelayMS(500);
        bsp_LedToggle(1);
        bsp_LedToggle(2);
        bsp_LedToggle(3);
        if(g_RstCount++>1000)
             SoftReset();
    }
    g_RstCount =0 ;
    bsp_StartAutoTimer(0,2000);
    IWDG_Init(5,1250);
	while (1)
	{
      
        IWDG_Feed();
        if(g_hcStable)
            MODS_Poll();
		HC_Analyze();

       // if(g_hcRevOK)
        if(bsp_CheckTimer(0))
        {
            HC_AskForData();
          //  g_hcRevOK =0;
           // bsp_DelayMS(1000);
            g_RstCount++; //复位计数器++
            if(g_RstCount >60)
                 SoftReset();
        }
        
	}
}

void SoftReset(void)
{
        __set_FAULTMASK(1);
        NVIC_SystemReset();
}







