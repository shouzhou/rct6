#include "bsp.h"
#include "modbus_slave.h"
//uint8_t send[10]="{F04RDD}";
//uint8_t send[10]={0x7B,0x46,0x30,0x34,0x52,0x44,0x44,0x7D,0x0D}; //0x0D ����carriage return
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
extern uint16_t g_RstCount;
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
    
    
//    g_hum  = (g_tHC.RxBuf[5]-0x30)*1000+(g_tHC.RxBuf[6]-0x30)*100+(g_tHC.RxBuf[8]-0x30)*10+(g_tHC.RxBuf[9]-0x30);
//    g_temp = (g_tHC.RxBuf[22]-0x30)*1000+(g_tHC.RxBuf[23]-0x30)*100+(g_tHC.RxBuf[25]-0x30)*10+(g_tHC.RxBuf[26]-0x30);
    //�޸����ݽ��ղ��� ��ʹ�ñ����λ���ݺͰ�λ���ݳ�������20200325
    g_hum  =( (g_tHC.RxBuf[4]==0x20)? 0:(g_tHC.RxBuf[4]-0x30))*10000+\
            ((g_tHC.RxBuf[5]==0x20)? 0:(g_tHC.RxBuf[5]-0x30))*1000+\
            (g_tHC.RxBuf[6]-0x30)*100+(g_tHC.RxBuf[8]-0x30)*10+(g_tHC.RxBuf[9]-0x30);
             
    g_temp = ((g_tHC.RxBuf[21]==0x20)? 0:(g_tHC.RxBuf[21]-0x30))*10000+\
            ((g_tHC.RxBuf[22]==0x20)? 0:(g_tHC.RxBuf[22]-0x30))*1000+\
            (g_tHC.RxBuf[23]-0x30)*100+(g_tHC.RxBuf[25]-0x30)*10+(g_tHC.RxBuf[26]-0x30);
	
  
   if((g_hcDataOk==0)&&(g_hum<15000)&&(g_temp<15000)) //�״����� ���ȴ�5������
   {
        g_humArray[g_hcRevCount] = g_hum;
        g_tempArray[g_hcRevCount++] = g_temp;
   } 
   else if((g_hcDataOk==1)&&(g_hum<15000)&&(g_temp<15000)) //����5�κ󣬿�ʼ��ÿһ�����ݽ�����������
   {
        bsp_HCDataProcess(g_hum,g_temp);   
        bsp_LedToggle(3); //���������ݽ���һ�� ��˸һ��	D6	
        g_RstCount =0; //��λ����������
   }
			
	
err_ret:
#if 0										/* �˲���Ϊ�˴��ڴ�ӡ���,ʵ�������пɲ�Ҫ */
	g_tPrint.Rxlen = g_tHC.RxCount;
	memcpy(g_tPrint.RxBuf, g_tHC.RxBuf, g_tHC.RxCount);
#endif
	
	g_tHC.RxCount = 0;					/* ��������������������´�֡ͬ�� */
    g_hum =20000;
    g_temp =20000;
    g_hcRevOK =1; //��ʾ���ܽ���ok
//   g_hcRevCount++;
   if(g_hcRevCount>4) //�տ��� ���ݴ���5����Ч��
   {
       g_hcRevCount =0;
       if(g_hcDataOk ==0)
            g_hcDataOk =1;
       
   }
}


//uint16_t g_hum,g_temp;
//uint16_t g_humArray[5],g_tempArray[5]; //�ݴ�
//uint16_t g_humFinal,g_tempFinal; //485 ��ѯ���
//uint16_t g_humAbnormal[5],g_tempAbnormal[5]; //�쳣
//uint8_t g_hcRevOK=0,g_hcRevCount=0; // ���յ���־  + �ܴ���
void bsp_HCDataProcess(uint16_t hum,uint16_t temp)
{
    static uint8_t indexhum=0,indextemp=0; //����������Ҫ������±� 0-4
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
////////////////////ʪ�ȲŴ���///////////////////    
    if(abs(hum-tphum)>500) //���������
    {
        g_humAbnormal[indexabhum++] = hum;
    }
    else //����û�����
    {
        g_humArray[indexhum++] = hum;
        indexabhum=0; //��������±�����
         g_tVar.P01 = hum; //����modbus����
    }
/////////////////////�¶ȴ���/////////////////////
     if(abs(temp-tptemp)>100) //���������
    {
        g_tempAbnormal[indexabtemp++] = temp;
    }
    else //����û�����
    {
        g_tempArray[indextemp++] = temp;
        indexabtemp=0; //��������±�����
        g_tVar.P02 = temp; //����modbus����
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
