/********************************************************/
// CPU需要：STM32F103--RAM内存不小于48K	Flash内存不小于256K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20160917
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************
20151023：解决远程运行几次才RUN的问题
20151125：解决用三菱软件下载程序过程中就运行的问题
20151204：解决当PLC为STOP时，将T、D、C以及M清零，与FX2N的PLC一致
20160423：新增在线监控写入功能函数，并调试通过
20160510：新增掉电保存功能，并调试测试通过。优化程序架构
20160609：新增485 modbus的rtc通信协议，使用usart3
20160622： D8121从站地址号默认是1；D8130主从站选择 0：从站
20160710: 新增1路高速脉冲输出Y0，PLSY
20160914：新增RTC时钟功能
					星期D8019、年D8018、月D8017、日D8016、时D8015、分D8014、 秒D8013
20160929：新增循环FOR、循环结束NEST	2条指令		
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
  PLC_IO_config();		                       // PLC输入输出初始化 
  RTC_Init();                                // RTC时钟配置	
  data_init();		                           // 调用D8000~D8126初始化
  Recover_data();                            // PLC断电数据恢复
  USART1_Configuration();                    // 串口初始化 
  USART3_Configuration();
  PLC_Timer();	                             // TIMER2 定时器初始化  里面加入一项单相输入编码器
  ADC_init();	                               // ADC初始化
  NCIC_Confinguration(ENABLE);               // 开启中断程序全部等级划
  while(1)                                      
  {	
     Err_Lamp_OFF;	
//      RST_C();                              // 高速计数器
     RTC_Get();		                            // 时间扫描程序
		
	   PLC_ProInstructParse();    	            // PLC指令解析	
		
		 mosbus_485_USART3();
                            		
	   if(Send_out)
				TX_Process();	                        // 发送串口数据 
    
		//*	低电压检测  断电保持数据
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
				PLC_16BIT[80].bytes = 0;                 //关闭所有IO输出
				PLC_DATA_KEEP(); 
				NCIC_Confinguration(DISABLE);            //关闭中断程序全部等级划分	
				Timer1[0]=0;
			}
		}	 
//*/ 
// 	 if(power_down==0)                         //判断是不是断电
//     {
// 		   NCIC_Confinguration(DISABLE);         //关闭中断程序全部等级划分	
// 	     PLC_DATA_KEEP(); 
// 	     while(1){;}
//     }  	
  }
}  
