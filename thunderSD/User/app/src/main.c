/*
*********************************************************************************************************
*
//LED1 -PB6--�����ź�-�ŵ�EXT��
    //LED2-PB7 --�����ź� �ŵ�ad��  
*   
*********************************************************************************************************
*/

#include "bsp.h"			/* �ײ�Ӳ������ */
#include "demo_sd_fatfs.h"
int main(void)
{

	bsp_Init();							/* Ӳ����ʼ�� */
 
	while (1)
	{
        if(bsp_CheckTimer(1))
        {
           bsp_LedToggle(2);
          
        }

        if(bsp_CheckTimer(0))
        {
            bsp_StopTimer(1);
            bsp_LedOff(2);
            StartADC();	//�ٴο���ADC  ��ʼ�´β���
            
        }
	}
}






