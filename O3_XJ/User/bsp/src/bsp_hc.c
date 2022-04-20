#include "bsp.h"
#define PASSWORD   123456   //密码值 6位
static uint8_t g_hc_timeout = 0;
uint32_t g_password =0;
uint16_t g_lenth=0,g_width=0,g_height=0;
uint16_t g_funcTouch = 0xff , g_funcKey = 0xff;
uint8_t g_hcRevOK=0,g_FailTime=0;
HC_T g_tHC;


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
   // printf("in\r\n");
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

    0e--g_plant
    0f--g_chemical
    
*/

//demo
/*
A5 5A 08 83 00 07 02 00 01 E2 40  -- 密码屏返回值   123456   01 e2 40 
A5 5A 06 83 00 01 01 02 33  --长度  56.3   233
A5 5A 06 83 00 02 01 02 33 --宽度 56.3     233
A5 5A 06 83 00 03 01 03 E8 --高度 100





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
        if(g_tHC.RxBuf[5] ==0x12)  //0x0001 --对应长度
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
        if(g_tHC.RxBuf[5] ==0x13)  //0x0002 --对应宽度
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
        if(g_tHC.RxBuf[5] ==0x14)  //0x0003 --对应高度
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
        // A5 5A 08 83 00 07 02 00 01 E2 40  -- 密码屏返回值   123456   01 e2 40
        if(g_tHC.RxBuf[5] ==0x01)  //0x0007 --对应密码
        {
            g_password = g_tHC.RxBuf[8]*65536+g_tHC.RxBuf[9]*256+g_tHC.RxBuf[10];
            if(g_password == PASSWORD) //密码正确
            {
                g_touch = 1;
             //   bsp_DelayMS(100);
               // bsp_SendKey(3); //跳转到设置体积
               // bsp_DelayMS(5);
               // bsp_DisableTouch();//关闭触摸
            }
            else //密码错误 回到状态界面
            {
              //  bsp_SendKey(4); //回到状态界面
               // bsp_DelayMS(5);
              //  bsp_DisableTouch();//关闭触摸
                g_touch = 2;
            }
            
            #ifdef USEDEBUG
            
              printf("g_password = %ld\r\n",g_password);
              g_password =0;
             
            #endif 
        }
        // 开启和关闭各种模式--begin
        //页面   功能           地址   字长   功能              对应变量值
        //3	    启动手动模式	0x000A	1	确定设置值   			1
        //3	    启动循环模式	0x000A	1	开启数据上传			2
        //3	    启动自动浓度	0x000A	1	关闭数据上传			3
                
        if(g_tHC.RxBuf[5] ==0x0A)  //按键键值返回，对应开启和关闭功能
        {
            g_funcTouch = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            
            if(g_funcTouch ==1 )  //确定
            {
               // bsp_DisableTouch(); //关闭触摸
                g_touch= 3;
                
            }
            else if(g_funcTouch ==2)
            {
               g_WithoutOnenet = 0; //开启数据上传 
             //  bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
                g_touch = 4;
            }
            else if(g_funcTouch ==3)
            {
               g_WithoutOnenet = 1; //关闭数据上传 
             //  bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
                g_touch = 5;
            }
           
            #ifdef USEDEBUG
            
            printf("g_funcTouch = %d\r\n",g_funcTouch);
             g_funcTouch = 0xff;  //处理完毕，恢复原始值
            #endif 
          //  bsp_LedFlash(g_func+1);
        }
    }

			
	
err_ret:
#if 0										/* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Rxlen = g_tHC.RxCount;
	memcpy(g_tPrint.RxBuf, g_tHC.RxBuf, g_tHC.RxCount);
#endif
	
	g_tHC.RxCount = 0;					/* 必须清零计数器，方便下次帧同步 */
    g_hcRevOK =1; //表示接受解码ok
}
