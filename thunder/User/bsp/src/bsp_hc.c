#include "bsp.h"
#include "modbus_slave.h"
//uint8_t send[10]="{F04RDD}";
uint8_t send[10]={0x7B,0x46,0x30,0x34,0x52,0x44,0x44,0x7D,0x0D}; //0x0D ����carriage return

static uint8_t g_hc_timeout = 0;
uint16_t g_hum,g_temp;
uint8_t g_hcRevOK=0;
HC_T g_tHC;

/*********************************************************************************************************
*	�� �� ��: HC_RxTimeOut
*	����˵��: ����3.5���ַ�ʱ���ִ�б������� ����ȫ�ֱ��� g_hc_timeout = 1; ֪ͨ������ʼ���롣
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void HC_RxTimeOut(void)
{
	g_hc_timeout = 1;
}
void HC_ReciveNew(uint8_t _data)
{
        /*
		3.5���ַ���ʱ������ֻ������RTUģʽ���棬��ΪRTUģʽû�п�ʼ���ͽ�������
		�������ݰ�֮��ֻ�ܿ�ʱ���������֣�Modbus�����ڲ�ͬ�Ĳ������£����ʱ���ǲ�һ���ģ�
		���Ծ���3.5���ַ���ʱ�䣬�����ʸߣ����ʱ������С�������ʵͣ����ʱ������Ӧ�ʹ�

		4800  = 7.297ms
		9600  = 3.646ms
		19200  = 1.771ms
		38400  = 0.885ms
	*/
	uint32_t timeout;

	g_hc_timeout = 0;
	
	timeout = 45000000 / UART2_BAUD;		/* ���㳬ʱʱ�䣬��λus 35000000*/
	
	/* Ӳ����ʱ�жϣ���ʱ����us Ӳ����ʱ��2����MODBUS�ӻ�, ��ʱ��3����MODBUS�ӻ�����*/
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
void HC_Analyze(void)
{
    if (g_hc_timeout == 0)	
	{
		return;								/* û�г�ʱ���������ա���Ҫ���� g_tHC.RxCount */
	}
	
	g_hc_timeout = 0;	 					/* ���־ */

	if (g_tHC.RxCount < 10)				/* ���յ�������С�ڸ��ֽھ���Ϊ���� */
	{
		goto err_ret;
	}

//	g_hum  = (g_tHC.RxBuf[13]-0x30)*1000+(g_tHC.RxBuf[14]-0x30)*100+(g_tHC.RxBuf[16]-0x30)*10+(g_tHC.RxBuf[17]-0x30);
//    g_temp = (g_tHC.RxBuf[30]-0x30)*1000+(g_tHC.RxBuf[31]-0x30)*100+(g_tHC.RxBuf[33]-0x30)*10+(g_tHC.RxBuf[34]-0x30);
    g_hum  = (g_tHC.RxBuf[5]-0x30)*1000+(g_tHC.RxBuf[6]-0x30)*100+(g_tHC.RxBuf[8]-0x30)*10+(g_tHC.RxBuf[9]-0x30);
    g_temp = (g_tHC.RxBuf[22]-0x30)*1000+(g_tHC.RxBuf[23]-0x30)*100+(g_tHC.RxBuf[25]-0x30)*10+(g_tHC.RxBuf[26]-0x30);
	
    if(g_hum<10000)
    g_tVar.P01 = g_hum ;
    if(g_temp<10000)
    g_tVar.P02 = g_temp;
	 bsp_LedToggle(3); //���������ݽ���һ�� ��˸һ��	D6			
	
err_ret:
#if 1										/* �˲���Ϊ�˴��ڴ�ӡ���,ʵ�������пɲ�Ҫ */
	g_tPrint.Rxlen = g_tHC.RxCount;
	memcpy(g_tPrint.RxBuf, g_tHC.RxBuf, g_tHC.RxCount);
#endif
	
	g_tHC.RxCount = 0;					/* ��������������������´�֡ͬ�� */
    g_hcRevOK =1; //��ʾ���ܽ���ok
}
