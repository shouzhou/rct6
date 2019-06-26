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

int main(void)
{
    uint8_t keyvalue=0;
	bsp_Init();							/* 硬件初始化 */
	
//	bsp_StartAutoTimer(1,200);
	while (1)
	{
        keyvalue = bsp_KeyScan(0);	
        if(keyvalue ==3)
            
        {
          
          bsp_StartTimer(0,3000);
          bsp_StartAutoTimer(1,100);            
             
        }
        if(bsp_CheckTimer(1))
        {
           bsp_LedToggle(2);
          
        }

        if(bsp_CheckTimer(0))
        {
            bsp_StopTimer(1);
        }
	}
}







