
// �ó��� ��onenet������ �������޸� ���������Լ���RCT6���������У�ģ���ϴ���ʪ�� �Լ�����onenet����led ����

#include "bsp.h"



 
int main(void)
{
   

    uint8_t tempsec=0;
    uint8_t ucKeyCode;
    float  ADCConvertedValueLocal[4]={0.0};
   
    
     bsp_Init();
    
     bsp_SysCheck();//ϵͳ�Լ죬������ɹ����򵯳������Ի���
     g_status = SYSINIFINISH; //��ʼ�����
    // g_touch = 13; //���½���
    
    bsp_ScreenChange(3);
    while(1)
    {
       // printf("while(1)\r\n");
        IWDG_Feed();
        
        ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* �м����� */
			switch (ucKeyCode)
			{
				case KEY_LONG_K1:		/* ����K1������ */
					printf("����K1����\r\n");
					bsp_LedToggle(1);	
					break;		
                case KEY_DOWN_K1:		/* ����K1������ */
					printf("����K1����\r\n");
					bsp_LedToggle(1);	
					break;
				
				case KEY_DOWN_K2:		/* ����K2������ */
					printf("����K2����\r\n");
					bsp_LedToggle(3);					
					break;

				case SYS_DOWN_K1K2:		/* ����K3������ */
					printf("���K1K2����\r\n");	
					bsp_LedToggle(4);	
					break;				
				case KEY_LONG_K3:		/* ����K1������ */
					printf("����K3����\r\n");
					bsp_LedToggle(1);	
					break;	
                
                case KEY_LONG_K4:		/* ����K1������ */
					printf("����K4����\r\n");
					bsp_LedToggle(1);	
					break;	
				default:
					break;
			}
		}
        
        
        
        HC_Analyze();
        RTC_ReadClock();	/* ��ʱ�ӣ���������ȫ�ֱ��� g_tRTC */

      //  FunTouch();
        if(tempsec != g_tRTC.Sec)
        {
        if(g_tRTC.Sec%10 ==0)
        {
            printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day,g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
            ADCConvertedValueLocal[0]=(float)ADCConvertedValue[0]/4096*3.3;
            printf(" \r\n ͨ��10(PC0)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[0]);
            
        }
        tempsec=g_tRTC.Sec;
        //  if(g_tRTC.Sec %10 ==0)
        //  bsp_ScreenUpdateText();
        FunSwitch();
        bsp_LedToggle(1);

        #ifdef USEDEBUG
//                       ADCConvertedValueLocal[0]=(float)ADCConvertedValue[0]/4096*3.3;
//                       printf(" \r\n ͨ��10(PC0)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[0]);
        //               ADCConvertedValueLocal[1]=(float)ADCConvertedValue[1]/4096*3.3;
        //               printf(" \r\n ͨ��11(PC1)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[1]);
        //               ADCConvertedValueLocal[2]=(float)ADCConvertedValue[2]/4096*3.3;
        //               printf(" \r\n ͨ��12(PC2)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[2]);
        //               ADCConvertedValueLocal[3]=(float)ADCConvertedValue[3]/4096*3.3;
        //               printf(" \r\n ͨ��13(PC3)�ź�ת����ѹ�� %5f V \r\n",ADCConvertedValueLocal[3]);
        #endif
        }
           
		       
    } 

}
