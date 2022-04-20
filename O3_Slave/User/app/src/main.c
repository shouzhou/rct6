#include "bsp.h"			/* �ײ�Ӳ������ */
#include "modbus_slave.h"
#include "main.h"






PRINT_MODS_T g_tPrint;

/*
*********************************************************************************************************
*	�� �� ��: main
*	����˵��: c�������
*	��    �Σ���
*	�� �� ֵ: �������(���账��)
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




