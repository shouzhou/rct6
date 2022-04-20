#include "bsp.h"
static uint8_t g_hc_timeout = 0;

uint16_t g_lenth=0,g_width=0,g_height=0;
uint16_t g_func=0xff;
uint16_t g_mannualtime =0,g_loopruntime =0 ,g_loopstoptime =0;
uint8_t  g_loopstatus =1; //0 �����ʱ��� ��1 ����ʱ���
uint8_t  g_AutoNh3Status =1;
uint8_t g_hcRevOK=0,g_FailTime=0;
HC_T g_tHC;



//0	�����ֶ�ģʽ	0x000A	1	�����ֶ�ģʽ			0
//0	����ѭ��ģʽ	0x000A	1	����ѭ��ģʽ			1
//4	�����Զ�Ũ��	0x000A	1	�����Զ�Ũ��			3
//4	ֹͣ�Զ�Ũ��	0x000A	1	ֹͣ�Զ�Ũ��			4
//4	�����Զ����	0x000A	1	�����Զ����			5
//4	ֹͣ�Զ����	0x000A	1	ֹͣ�Զ����			6
//3	ֹͣ�ֶ�ģʽ	0x000A	1	ֹͣ�ֶ�ģʽ			2
//5	ֹͣѭ��ģʽ	0x000A	1	ֹͣѭ��ģʽ			7




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
	
	timeout = 45000000 / 115200;		/* ���㳬ʱʱ�䣬��λus 35000000*/
	
	/* Ӳ����ʱ�жϣ���ʱ����us Ӳ����ʱ��2����MODBUS�ӻ�, ��ʱ��3����MODBUS�ӻ�����*/
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

// ���أ� 0xA5 0x5A 0x05 0x83 0x00 0x03 0x01 0x00 
//   A5 5A 06 83 00 00 01 03 E8    === 1000
/*
    EEPROM �����ַ  ��6���ֽڴ�ŵ�ǰ����ʪ��
    00 --0X00      
    //  ��0λ ��ʾ ��������ok
    //  ��1λ ��ʾ �������OK
    //  ��2λ ��ʾ �߶�����OK
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
		return;								/* û�г�ʱ���������ա���Ҫ���� g_tHC.RxCount */
	}
	
	g_hc_timeout = 0;	 					/* ���־ */

	if (g_tHC.RxCount < 5)				/* ���յ�������С�ڸ��ֽھ���Ϊ���� */
	{
		goto err_ret;
	}
    if((g_tHC.RxBuf[0] == 0xA5)&&(g_tHC.RxBuf[1] == 0x5A)&&(g_tHC.RxBuf[3] == 0x83)) //���ڱ����Զ��ϴ�
    {
        //  �����--begin
        if(g_tHC.RxBuf[5] ==0x10)  //0x0010 --��Ӧ����
        {
            g_lenth = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[1] = g_tHC.RxBuf[7];
            eeResult[2] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x01;             //��־ �����Ѿ�����
            ee_WriteBytes(0,eeResult,0x00,0x01); //д��־
            ee_WriteBytes(0,&eeResult[1],0x01,0x02); //д����
            #ifdef USEDEBUG
            
              printf("legth = %d\r\n",g_lenth/10);
            
            #endif 
        }
        if(g_tHC.RxBuf[5] ==0x11)  //0x0011 --��Ӧ���
        {
            g_width= ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            
            eeResult[3] = g_tHC.RxBuf[7];
            eeResult[4] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x02;             //��־ �����Ѿ�����
            ee_WriteBytes(0,eeResult,0x00,0x01); //д��־
            ee_WriteBytes(0,&eeResult[3],0x03,0x02); //д����
            
            #ifdef USEDEBUG
            
              printf("width = %d\r\n",g_width/10);
            
            #endif 
            
        }
        if(g_tHC.RxBuf[5] ==0x12)  //0x0012 --��Ӧ�߶�
        {
            g_height = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[5] = g_tHC.RxBuf[7];
            eeResult[6] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x04;             //��־ �����Ѿ�����
            ee_WriteBytes(0,eeResult,0x00,0x01); //д��־
            ee_WriteBytes(0,&eeResult[5],0x05,0x02); //д����
            
            #ifdef USEDEBUG
            
              printf("height = %d\r\n",g_height/10);
            
            #endif 
        }
        //  �����--end
        
        //����ʱ��----begin
        if(g_tHC.RxBuf[5] ==0x00)  //0x0000 --�ֶ�����ʱ��
        {
            g_mannualtime = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[8] = g_tHC.RxBuf[7];
            eeResult[9] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x08;             //��־ �����Ѿ�����
            ee_WriteBytes(0,eeResult,0x00,0x01); //д��־
            ee_WriteBytes(0,&eeResult[8],0x08,0x02); //д����
            
            #ifdef USEDEBUG
            
              printf("mannualtime = %d\r\n",g_mannualtime);
            
            #endif 
        }
        
        if(g_tHC.RxBuf[5] ==0x01)  //0x0001 --ѭ������ʱ��
        {
            g_loopruntime = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[10] = g_tHC.RxBuf[7];
            eeResult[11] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x10;             //��־ �����Ѿ�����
            ee_WriteBytes(0,eeResult,0x00,0x01); //д��־
            ee_WriteBytes(0,&eeResult[10],0x0a,0x02); //д����
            
            #ifdef USEDEBUG
            
              printf("loopruntime = %d\r\n",g_loopruntime);
            
            #endif 
        }
        
        
        if(g_tHC.RxBuf[5] ==0x02)  //0x0002 --ѭ��ֹͣʱ��
        {
            g_loopstoptime = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            eeResult[12] = g_tHC.RxBuf[7];
            eeResult[13] = g_tHC.RxBuf[8];     
            eeResult[0] |= 0x20;             //��־ �����Ѿ�����
            ee_WriteBytes(0,eeResult,0x00,0x01); //д��־
            ee_WriteBytes(0,&eeResult[12],0x0c,0x02); //д����
            
            #ifdef USEDEBUG
            
              printf("loopstoptime = %d\r\n",g_loopstoptime);
            
            #endif 
        }
        
        
        
        //����ʱ�� ---end
        
        
        // �����͹رո���ģʽ--begin
        //ҳ��   ����           ��ַ   �ֳ�   ����              ��Ӧ����ֵ
        //0	    �����ֶ�ģʽ	0x000A	1	�����ֶ�ģʽ			0
        //0	    ����ѭ��ģʽ	0x000A	1	����ѭ��ģʽ			1
        //4	    �����Զ�Ũ��	0x000A	1	�����Զ�Ũ��			3
        //4	    ֹͣ�Զ�Ũ��	0x000A	1	ֹͣ�Զ�Ũ��			4
        //4	    �����Զ����	0x000A	1	�����Զ����			5
        //4	    ֹͣ�Զ����	0x000A	1	ֹͣ�Զ����			6
        //3	    ֹͣ�ֶ�ģʽ	0x000A	1	ֹͣ�ֶ�ģʽ			2
        //5	    ֹͣѭ��ģʽ	0x000A	1	ֹͣѭ��ģʽ			7
        
        //8	    ��������ģʽ	0x000A	1	��������ģʽ		    8
        //8	    �������ģʽ	0x000A	1	�������ģʽ			9
        if(g_tHC.RxBuf[5] ==0x0A)  //������ֵ���أ���Ӧ�����͹رչ���
        {
            g_func = ((uint16_t)g_tHC.RxBuf[7]<<8)+g_tHC.RxBuf[8];
            
            
            #ifdef USEDEBUG
            
            printf("g_func = %d\r\n",g_func);
            
            #endif 
          //  bsp_LedFlash(g_func+1);
        }
        
        // �����͹رո���ģʽ--end
    }

			
	
err_ret:
#if 0										/* �˲���Ϊ�˴��ڴ�ӡ���,ʵ�������пɲ�Ҫ */
	g_tPrint.Rxlen = g_tHC.RxCount;
	memcpy(g_tPrint.RxBuf, g_tHC.RxBuf, g_tHC.RxCount);
#endif
	
	g_tHC.RxCount = 0;					/* ��������������������´�֡ͬ�� */
    g_hcRevOK =1; //��ʾ���ܽ���ok
}
