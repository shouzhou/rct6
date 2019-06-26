/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*   ϵͳ ����LED D4 D5 D6  
*   D4 ÿ0.5�� ��˸һ�� ��ʾϵͳ��������
*   D5 modbus ÿͨѶһ�� ����˸һ�� ��ͨѶ���ڲ���˸״̬
*   D6 ������ÿ����һ�����ݲ����յ��� ��˸һ�� ����������˸

*   ����modbus�ӻ���ַ�� ��Ҫ��������������Ч
*   
*********************************************************************************************************
*/

#include "bsp.h"			/* �ײ�Ӳ������ */
#include "modbus_slave.h"
#include "main.h"


PRINT_MODS_T g_tPrint;
extern VAR_T g_tVar;

extern uint8_t send[10];
extern uint32_t g_iRunTime;
void SoftReset(void);
int main(void)
{
	bsp_Init();							/* Ӳ����ʼ�� */
	SADDR485 = bsp_Read485Address(); 
    
            
    HC_AskForData();
	while(!g_hcRevOK) //�ȴ�̽ͷ����
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







