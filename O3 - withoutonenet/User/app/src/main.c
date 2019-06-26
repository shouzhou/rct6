
#include "bsp.h"			/* 底层硬件驱动 */
DHT11_T  g_DHT11;
uint16_t temp ,humi;
int main(void)
{
    uint8_t keyvalue;
    uint16_t  count=0;
    uint8_t ret,flag=0;
	bsp_Init();							/* 硬件初始化 */
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







