/********************************************************/
// CPU��Ҫ��STM32F103--RAM�ڴ治С��48K	Flash�ڴ治С��256K
// ����������STM32F103RDT6��VET6����ͨ��
// �༭���ڣ�20160917
// editor by СС��
// ���꣺shop182385147.taobao.com
/********************************************************
20151023�����Զ�����м��β�RUN������
20151125�����������������س�������о����е�����
20151204�������PLCΪSTOPʱ����T��D��C�Լ�M���㣬��FX2N��PLCһ��
20160423���������߼��д�빦�ܺ�����������ͨ��
20160510���������籣�湦�ܣ������Բ���ͨ�����Ż�����ܹ�
20160609������485 modbus��rtcͨ��Э�飬ʹ��usart3
20160622�� D8121��վ��ַ��Ĭ����1��D8130����վѡ�� 0����վ
20160710: ����1·�����������Y0��PLSY
20160914������RTCʱ�ӹ���
					����D8019����D8018����D8017����D8016��ʱD8015����D8014�� ��D8013
20160929������ѭ��FOR��ѭ������NEST	2��ָ��		
********************************************************/

#include "main.h"
#include "PLC_Dialogue.h"

u8 power_down;
u8 run_flag;
u16 Timer1[2];
extern bit_byte PLC_16BIT[12100];
extern void mosbus_485_USART3(void);

int main(void)
{ 
	
  power_down=10;
  PLC_IO_config();		                       // PLC���������ʼ�� 
  RTC_Init();                                // RTCʱ������	
  data_init();		                           // ����D8000~D8126��ʼ��
  Recover_data();                            // PLC�ϵ����ݻָ�
  USART1_Configuration();                    // ���ڳ�ʼ�� 
  USART3_Configuration();
  PLC_Timer();	                             // TIMER2 ��ʱ����ʼ��  �������һ������������
  ADC_init();	                               // ADC��ʼ��
  NCIC_Confinguration(ENABLE);               // �����жϳ���ȫ���ȼ���
  while(1)                                      
  {	
     Err_Lamp_OFF;	
//      RST_C();                              // ���ټ�����
     RTC_Get();		                            // ʱ��ɨ�����
		
	   PLC_ProInstructParse();    	            // PLCָ�����	
		
		 mosbus_485_USART3();
                            		
	   if(Send_out)
				TX_Process();	                        // ���ʹ������� 
    
		//*	�͵�ѹ���  �ϵ籣������
		if(!PVD)	   //MY PCB== !PVD
		{	
			if(Timer1[0]==0)
			Recover_data();  
			if(Timer1[0]<=60000)
			Timer1[0]++;
		}
		else
		{
// 			all_data[0x180/2]=0;
			if(Timer1[0]>=100)
			{
				PLC_16BIT[80].bytes = 0;                 //�ر�����IO���
				PLC_DATA_KEEP(); 
				NCIC_Confinguration(DISABLE);            //�ر��жϳ���ȫ���ȼ�����	
				Timer1[0]=0;
			}
		}	 
//*/ 
// 	 if(power_down==0)                         //�ж��ǲ��Ƕϵ�
//     {
// 		   NCIC_Confinguration(DISABLE);         //�ر��жϳ���ȫ���ȼ�����	
// 	     PLC_DATA_KEEP(); 
// 	     while(1){;}
//     }  	
  }
}  
