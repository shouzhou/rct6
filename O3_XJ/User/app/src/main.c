
// 该程序 在onenet基础上 进行了修改 ，可以在自己的RCT6板子上运行，模拟上传温湿度 以及利用onenet进行led 控制

#include "bsp.h"



 
int main(void)
{
   

    uint8_t tempsec=0;
    uint8_t ucKeyCode;
    float  ADCConvertedValueLocal[4]={0.0};
   
    
     bsp_Init();
    
     bsp_SysCheck();//系统自检，如果不成功，则弹出报警对话框
     g_status = SYSINIFINISH; //初始化完成
    // g_touch = 13; //更新界面
    
    bsp_ScreenChange(3);
    while(1)
    {
       // printf("while(1)\r\n");
        IWDG_Feed();
        
        ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_LONG_K1:		/* 按键K1键按下 */
					printf("按键K1长按\r\n");
					bsp_LedToggle(1);	
					break;		
                case KEY_DOWN_K1:		/* 按键K1键按下 */
					printf("按键K1按下\r\n");
					bsp_LedToggle(1);	
					break;
				
				case KEY_DOWN_K2:		/* 按键K2键按下 */
					printf("按键K2按下\r\n");
					bsp_LedToggle(3);					
					break;

				case SYS_DOWN_K1K2:		/* 按键K3键按下 */
					printf("组合K1K2按下\r\n");	
					bsp_LedToggle(4);	
					break;				
				case KEY_LONG_K3:		/* 按键K1键按下 */
					printf("按键K3长按\r\n");
					bsp_LedToggle(1);	
					break;	
                
                case KEY_LONG_K4:		/* 按键K1键按下 */
					printf("按键K4长按\r\n");
					bsp_LedToggle(1);	
					break;	
				default:
					break;
			}
		}
        
        
        
        HC_Analyze();
        RTC_ReadClock();	/* 读时钟，结果存放在全局变量 g_tRTC */

      //  FunTouch();
        if(tempsec != g_tRTC.Sec)
        {
        if(g_tRTC.Sec%10 ==0)
        {
            printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day,g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
            ADCConvertedValueLocal[0]=(float)ADCConvertedValue[0]/4096*3.3;
            printf(" \r\n 通道10(PC0)信号转换电压： %5f V \r\n",ADCConvertedValueLocal[0]);
            
        }
        tempsec=g_tRTC.Sec;
        //  if(g_tRTC.Sec %10 ==0)
        //  bsp_ScreenUpdateText();
        FunSwitch();
        bsp_LedToggle(1);

        #ifdef USEDEBUG
//                       ADCConvertedValueLocal[0]=(float)ADCConvertedValue[0]/4096*3.3;
//                       printf(" \r\n 通道10(PC0)信号转换电压： %5f V \r\n",ADCConvertedValueLocal[0]);
        //               ADCConvertedValueLocal[1]=(float)ADCConvertedValue[1]/4096*3.3;
        //               printf(" \r\n 通道11(PC1)信号转换电压： %5f V \r\n",ADCConvertedValueLocal[1]);
        //               ADCConvertedValueLocal[2]=(float)ADCConvertedValue[2]/4096*3.3;
        //               printf(" \r\n 通道12(PC2)信号转换电压： %5f V \r\n",ADCConvertedValueLocal[2]);
        //               ADCConvertedValueLocal[3]=(float)ADCConvertedValue[3]/4096*3.3;
        //               printf(" \r\n 通道13(PC3)信号转换电压： %5f V \r\n",ADCConvertedValueLocal[3]);
        #endif
        }
           
		       
    } 

}
