/*
*********************************************************************************************************
*
*	ģ������ : ������ģ�顣
*   ϵͳ ����LED D4 D5 D6  
*   D4 ÿ0.5�� ��˸һ�� ��ʾϵͳ��������
*   D5 modbus ÿͨѶһ�� ����˸һ�� ��ͨѶ���ڲ���˸״̬
*   D6 ������ÿ����һ�����ݲ����յ��� ��˸һ�� ����������˸(������2sͨѶһ��)

*   ����modbus�ӻ���ַ�� ��Ҫ��������������Ч
*   
*********************************************************************************************************
*/

#include "bsp.h"			/* �ײ�Ӳ������ */
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
    
	bsp_Init();							/* Ӳ����ʼ�� */
	SADDR485 = bsp_Read485Address(); 
    
     
    HC_AskForData();
	while(!g_hcDataOk) //�ȴ�̽ͷ����
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
            g_RstCount++; //��λ������++
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







