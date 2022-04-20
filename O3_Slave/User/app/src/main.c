#include "bsp.h"			/* 底层硬件驱动 */
#include "modbus_slave.h"
#include "main.h"






PRINT_MODS_T g_tPrint;

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
    uint8_t count=0;
	bsp_Init();						
	while (1)
	{
        if(secondflag)
        {
            count ++;
            secondflag =0;
            FunSwitch();
            if(count%5==0)
            {
               // count=0;
                if(DHT11_ReadData(&g_DHT11))
                {
                    g_tVar.P02 = g_DHT11.Hum;
                    g_tVar.P03 = g_DHT11.Temp;
                }
                g_tVar.P04 = SADDR485*1000+count;
                g_tVar.P05 = SADDR485*1000+count;
            }
        }
        IWDG_Feed();
		MODS_Poll();
	}
}




