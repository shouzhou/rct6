
#include "bsp.h"			/* �ײ�Ӳ������ */
DHT11_T  g_DHT11;
uint16_t temp ,humi;
int main(void)
{
    uint8_t keyvalue;
    uint16_t  count=0;
    uint8_t ret,flag=0;
	bsp_Init();							/* Ӳ����ʼ�� */
	while (1)
	{
      //  keyvalue = bsp_KeyScan(0);
        keyvalue = bsp_MkeyScan();
        if(keyvalue)
            {
                bsp_KeyProcess(keyvalue);
            }
        
	}
}







