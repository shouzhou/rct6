#include "bsp.h"
static uint8_t g_hc_timeout = 0;

uint16_t g_lenth=0,g_width=0,g_height=0;
uint16_t g_func=0xff;
uint16_t g_mannualtime =0,g_loopruntime =0 ,g_loopstoptime =0;
uint8_t  g_loopstatus =1; //0 代表关时间段 ，1 代表开时间段
uint8_t  g_AutoNh3Status =1;
uint8_t g_hcRevOK=0,g_FailTime=0;
HC_T g_tHC;



//0	启动手动模式	0x000A	1	启动手动模式			0
//0	启动循环模式	0x000A	1	启动循环模式			1
//4	启动自动浓度	0x000A	1	启动自动浓度			3
//4	停止自动浓度	0x000A	1	停止自动浓度			4
//4	启动自动体积	0x000A	1	启动自动体积			5
//4	停止自动体积	0x000A	1	停止自动体积			6
//3	停止手动模式	0x000A	1	停止手动模式			2
//5	停止循环模式	0x000A	1	停止循环模式			7




/*********************************************************************************************************
*	函 数 名: HC_RxTimeOut
*	功能说明: 超过3.5个字符时间后执行本函数。 设置全局变量 g_hc_timeout = 1; 通知主程序开始解码。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void HC_RxTimeOut(void)
{
	g_hc_timeout = 1;
}
void HC_ReciveNew(uint8_t _data)
{
        /*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		所以就是3.5个字符的时间，波特率高，这个时间间隔就小，波特率低，这个时间间隔相应就大

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
	uint32_t timeout;

	g_hc_timeout = 0;
	
	timeout = 45000000 / 115200;		/* 计算超时时间，单位us 35000000*/
	
	/* 硬件定时中断，定时精度us 硬件定时器2用于MODBUS从机, 定时器3用于MODBUS从机主机*/
	bsp_StartHardTimer(3, timeout, (void *)HC_RxTimeOut);

	if (g_tHC.RxCount < HC_RX_BUF_SIZE)
	{
		g_tHC.RxBuf[g_tHC.RxCount++] = _data;
	}
    
}

void HC_AskForData(void)
{
 //  comSendBuf(COM2, send,9);
    
}

// 返回： 0xA5 0x5A 0x05 0x83 0x00 0x03 0x01 0x00 
//   A5 5A 06 83 00 00 01 03 E8    === 1000
/*
    EEPROM 分配地址  ：6个字节存放当前的温湿度
    00 --0X00      
    //  第0位 表示 长度设置ok
    //  第1位 表示 宽度设置OK
    //  第2位 表示 高度设置OK
    01 --lengthH
    02 --lengthL
    03 --widthH
    04 --widthL
    05 --heightH
    06 --heightL
    07 --failtime

    08 --mannualtimeH
    09 --mannualtimeL
    0a --loopruntimeH
    0b --loopruntimeL
    0c --loopstoptimeH
    0d --loopstoptimeL
    
*/
 
void HC_Analyze(void)
{
    if (g_hc_timeout == 0)	
	{
		return;								/* 没有超时，继续接收。不要清零 g_tHC.RxCount */
	}
	
	g_hc_timeout = 0;	 					/* 清标志 */

	if (g_tHC.RxCount < 5)				/* 接收到的数据小于个字节就认为错误 */
	{
		goto err_ret;
	}
    if((g_tHC.RxBuf[0] == 0xA5)&&(g_tHC.RxBuf[1] == 0x5A)&&(g_tHC.RxBuf[3] == 0x83)) //属于变量自动上传
    {
        //  长宽高--begin
        if(g_tHC.RxBuf[5] ==0x10)  //0x0010 --对应长度
        {
            g_lenth = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[1] = g_tHC.RxBuf[7];
            eeResult[2] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x01;             //标志 长度已经设置
            ee_WriteBytes(0,eeResult,0x00,0x01); //写标志
            ee_WriteBytes(0,&eeResult[1],0x01,0x02); //写长度
            #ifdef USEDEBUG
            
              printf("legth = %d\r\n",g_lenth/10);
            
            #endif 
        }
        if(g_tHC.RxBuf[5] ==0x11)  //0x0011 --对应宽度
        {
            g_width= ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            
            eeResult[3] = g_tHC.RxBuf[7];
            eeResult[4] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x02;             //标志 长度已经设置
            ee_WriteBytes(0,eeResult,0x00,0x01); //写标志
            ee_WriteBytes(0,&eeResult[3],0x03,0x02); //写长度
            
            #ifdef USEDEBUG
            
              printf("width = %d\r\n",g_width/10);
            
            #endif 
            
        }
        if(g_tHC.RxBuf[5] ==0x12)  //0x0012 --对应高度
        {
            g_height = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[5] = g_tHC.RxBuf[7];
            eeResult[6] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x04;             //标志 长度已经设置
            ee_WriteBytes(0,eeResult,0x00,0x01); //写标志
            ee_WriteBytes(0,&eeResult[5],0x05,0x02); //写长度
            
            #ifdef USEDEBUG
            
              printf("height = %d\r\n",g_height/10);
            
            #endif 
        }
        //  长宽高--end
        
        //运行时间----begin
        if(g_tHC.RxBuf[5] ==0x00)  //0x0000 --手动开启时间
        {
            g_mannualtime = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[8] = g_tHC.RxBuf[7];
            eeResult[9] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x08;             //标志 长度已经设置
            ee_WriteBytes(0,eeResult,0x00,0x01); //写标志
            ee_WriteBytes(0,&eeResult[8],0x08,0x02); //写长度
            
            #ifdef USEDEBUG
            
              printf("mannualtime = %d\r\n",g_mannualtime);
            
            #endif 
        }
        
        if(g_tHC.RxBuf[5] ==0x01)  //0x0001 --循环开启时间
        {
            g_loopruntime = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[10] = g_tHC.RxBuf[7];
            eeResult[11] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x10;             //标志 长度已经设置
            ee_WriteBytes(0,eeResult,0x00,0x01); //写标志
            ee_WriteBytes(0,&eeResult[10],0x0a,0x02); //写长度
            
            #ifdef USEDEBUG
            
              printf("loopruntime = %d\r\n",g_loopruntime);
            
            #endif 
        }
        
        
        if(g_tHC.RxBuf[5] ==0x02)  //0x0002 --循环停止时间
        {
            g_loopstoptime = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[12] = g_tHC.RxBuf[7];
            eeResult[13] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x20;             //标志 长度已经设置
            ee_WriteBytes(0,eeResult,0x00,0x01); //写标志
            ee_WriteBytes(0,&eeResult[12],0x0c,0x02); //写长度
            
            #ifdef USEDEBUG
            
              printf("loopstoptime = %d\r\n",g_loopstoptime);
            
            #endif 
        }
        
        
        
        //运行时间 ---end
        
        
        // 开启和关闭各种模式--begin
        //页面   功能           地址   字长   功能              对应变量值
        //0	    启动手动模式	0x000A	1	启动手动模式			0
        //0	    启动循环模式	0x000A	1	启动循环模式			1
        //4	    启动自动浓度	0x000A	1	启动自动浓度			3
        //4	    停止自动浓度	0x000A	1	停止自动浓度			4
        //4	    启动自动体积	0x000A	1	启动自动体积			5
        //4	    停止自动体积	0x000A	1	停止自动体积			6
        //3	    停止手动模式	0x000A	1	停止手动模式			2
        //5	    停止循环模式	0x000A	1	停止循环模式			7
        
        //8	    进入消毒模式	0x000A	1	进入消毒模式		    8
        //8	    进入除臭模式	0x000A	1	进入除臭模式			9
        if(g_tHC.RxBuf[5] ==0x0A)  //按键键值返回，对应开启和关闭功能
        {
            g_func = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            
            
            #ifdef USEDEBUG
            
            printf("g_func = %d\r\n",g_func);
            
            #endif 
          //  bsp_LedFlash(g_func+1);
        }
        
        // 开启和关闭各种模式--end
    }

			
	
err_ret:
#if 0										/* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Rxlen = g_tHC.RxCount;
	memcpy(g_tPrint.RxBuf, g_tHC.RxBuf, g_tHC.RxCount);
#endif
	
	g_tHC.RxCount = 0;					/* 必须清零计数器，方便下次帧同步 */
    g_hcRevOK =1; //表示接受解码ok
}
