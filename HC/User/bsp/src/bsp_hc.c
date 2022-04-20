#include "bsp.h"
#include "modbus_slave.h"
//uint8_t send[10]="{F04RDD}";
//uint8_t send[10]={0x7B,0x46,0x30,0x34,0x52,0x44,0x44,0x7D,0x0D}; //0x0D 代表carriage return
uint8_t send[10]={0x7B,0x46,0x30,0x30,0x52,0x44,0x44,0x7D,0x0D};

static uint8_t g_hc_timeout = 0;
uint16_t g_hum=20000,g_temp=20000;
uint16_t g_humArray[6],g_tempArray[6];
//uint16_t g_humFinal,g_tempFinal;
uint16_t g_humAbnormal[6],g_tempAbnormal[6];
uint8_t g_hcRevOK=0,g_hcRevCount=0;
uint8_t g_hcStable =0,g_hcDataOk=0;
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
	uint32_t timeout;

	g_hc_timeout = 0;
	
	timeout = 45000000 / UART2_BAUD;		/* 计算超时时间，单位us 35000000*/
	
	/* 硬件定时中断，定时精度us 硬件定时器2用于MODBUS从机, 定时器3用于MODBUS从机主机*/
	bsp_StartHardTimer(3, timeout, (void *)HC_RxTimeOut);

	if (g_tHC.RxCount < HC_RX_BUF_SIZE)
	{
		g_tHC.RxBuf[g_tHC.RxCount++] = _data;
	}
    
}

void HC_AskForData(void)
{
   comSendBuf(COM2, send,9);
    
}
extern uint16_t g_RstCount;
void HC_Analyze(void)
{
    if (g_hc_timeout == 0)	
	{
		return;								/* 没有超时，继续接收。不要清零 g_tHC.RxCount */
	}
	
	g_hc_timeout = 0;	 					/* 清标志 */

	if (g_tHC.RxCount < 10)				/* 接收到的数据小于个字节就认为错误 */
	{
		goto err_ret;
	}

//	g_hum  = (g_tHC.RxBuf[13]-0x30)*1000+(g_tHC.RxBuf[14]-0x30)*100+(g_tHC.RxBuf[16]-0x30)*10+(g_tHC.RxBuf[17]-0x30);
//    g_temp = (g_tHC.RxBuf[30]-0x30)*1000+(g_tHC.RxBuf[31]-0x30)*100+(g_tHC.RxBuf[33]-0x30)*10+(g_tHC.RxBuf[34]-0x30);
    
    
//    g_hum  = (g_tHC.RxBuf[5]-0x30)*1000+(g_tHC.RxBuf[6]-0x30)*100+(g_tHC.RxBuf[8]-0x30)*10+(g_tHC.RxBuf[9]-0x30);
//    g_temp = (g_tHC.RxBuf[22]-0x30)*1000+(g_tHC.RxBuf[23]-0x30)*100+(g_tHC.RxBuf[25]-0x30)*10+(g_tHC.RxBuf[26]-0x30);
    //修改数据接收部分 ，使得避免个位数据和百位数据出现问题20200325
    g_hum  =( (g_tHC.RxBuf[4]==0x20)? 0:(g_tHC.RxBuf[4]-0x30))*10000+\
            ((g_tHC.RxBuf[5]==0x20)? 0:(g_tHC.RxBuf[5]-0x30))*1000+\
            (g_tHC.RxBuf[6]-0x30)*100+(g_tHC.RxBuf[8]-0x30)*10+(g_tHC.RxBuf[9]-0x30);
             
    g_temp = ((g_tHC.RxBuf[21]==0x20)? 0:(g_tHC.RxBuf[21]-0x30))*10000+\
            ((g_tHC.RxBuf[22]==0x20)? 0:(g_tHC.RxBuf[22]-0x30))*1000+\
            (g_tHC.RxBuf[23]-0x30)*100+(g_tHC.RxBuf[25]-0x30)*10+(g_tHC.RxBuf[26]-0x30);
	
  
   if((g_hcDataOk==0)&&(g_hum<15000)&&(g_temp<15000)) //首次启动 ，先存5次数据
   {
        g_humArray[g_hcRevCount] = g_hum;
        g_tempArray[g_hcRevCount++] = g_temp;
   } 
   else if((g_hcDataOk==1)&&(g_hum<15000)&&(g_temp<15000)) //收满5次后，开始对每一次数据进行跳动处理
   {
        bsp_HCDataProcess(g_hum,g_temp);   
        bsp_LedToggle(3); //传感器数据接收一次 闪烁一次	D6	
        g_RstCount =0; //复位计数器清零
   }
			
	
err_ret:
#if 0										/* 此部分为了串口打印结果,实际运用中可不要 */
	g_tPrint.Rxlen = g_tHC.RxCount;
	memcpy(g_tPrint.RxBuf, g_tHC.RxBuf, g_tHC.RxCount);
#endif
	
	g_tHC.RxCount = 0;					/* 必须清零计数器，方便下次帧同步 */
    g_hum =20000;
    g_temp =20000;
    g_hcRevOK =1; //表示接受解码ok
//   g_hcRevCount++;
   if(g_hcRevCount>4) //刚开机 数据大于5次有效了
   {
       g_hcRevCount =0;
       if(g_hcDataOk ==0)
            g_hcDataOk =1;
       
   }
}


//uint16_t g_hum,g_temp;
//uint16_t g_humArray[5],g_tempArray[5]; //暂存
//uint16_t g_humFinal,g_tempFinal; //485 查询结果
//uint16_t g_humAbnormal[5],g_tempAbnormal[5]; //异常
//uint8_t g_hcRevOK=0,g_hcRevCount=0; // 接收到标志  + 总次数
void bsp_HCDataProcess(uint16_t hum,uint16_t temp)
{
    static uint8_t indexhum=0,indextemp=0; //正常数据需要存入的下标 0-4
    static uint8_t indexabhum =0,indexabtemp =0;
    uint8_t i;
    uint16_t tphum=0,tptemp=0;
    //   g_humArray[g_hcRevCount]  =  hum;
    //   g_tempArray[g_hcRevCount] =  temp;
    for(i=0;i<5;i++)
    {
        tphum   +=  g_humArray[i];
        tptemp  +=  g_tempArray[i];
    }
    tphum  = tphum/5;
    tptemp = tptemp/5;
////////////////////湿度才处理///////////////////    
    if(abs(hum-tphum)>500) //数据有误差
    {
        g_humAbnormal[indexabhum++] = hum;
    }
    else //数据没有误差
    {
        g_humArray[indexhum++] = hum;
        indexabhum=0; //误差数据下标清零
         g_tVar.P01 = hum; //更新modbus数据
    }
/////////////////////温度处理/////////////////////
     if(abs(temp-tptemp)>100) //数据有误差
    {
        g_tempAbnormal[indexabtemp++] = temp;
    }
    else //数据没有误差
    {
        g_tempArray[indextemp++] = temp;
        indexabtemp=0; //误差数据下标清零
        g_tVar.P02 = temp; //更新modbus数据
        if(g_hcStable==0)
            g_hcStable = 1;
    }

    

    if(indexhum>4)  indexhum =0;
    if(indextemp>4) indextemp =0;
    
    if(indexabhum >4) 
    {
        indexabhum =0;
        g_tVar.P01 = (g_humAbnormal[0]+g_humAbnormal[1]+g_humAbnormal[2]+g_humAbnormal[3]+g_humAbnormal[4])/5;
        g_humArray[0] = g_tVar.P01;
        g_humArray[1] = g_tVar.P01;
        g_humArray[2] = g_tVar.P01;
        g_humArray[3] = g_tVar.P01;
        g_humArray[4] = g_tVar.P01;
    }
    
        if(indexabtemp >4) 
    {
        indexabtemp =0;
        g_tVar.P02= (g_tempAbnormal[0]+g_tempAbnormal[1]+g_tempAbnormal[2]+g_tempAbnormal[3]+g_tempAbnormal[4])/5;
        g_tempArray[0] = g_tVar.P02;
        g_tempArray[1] = g_tVar.P02;
        g_tempArray[2] = g_tVar.P02;
        g_tempArray[3] = g_tVar.P02;
        g_tempArray[4] = g_tVar.P02;
        
    }
}
