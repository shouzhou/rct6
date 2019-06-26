/*
*********************************************************************************************************
*
*	ģ������ : SD��Fat�ļ�ϵͳ��ʾģ�顣
*	�ļ����� : demo_sdio_fatfs.c
*	��    �� : V1.0
*	˵    �� : ��������ֲFatFS�ļ�ϵͳ���汾 R0.09b������ʾ��δ����ļ�����ȡ�ļ�������Ŀ¼��ɾ���ļ�
*			���������ļ���д�ٶȡ�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "ff.h"			/* FatFS�ļ�ϵͳģ��*/
#include "demo_sd_fatfs.h"

#define  NUM_BEFORE_EXT  500         //Ԥ���� �ڴ���ǰ��Ҫ���������ݸ���
extern uint16_t Ch1Buf[2048];	 /* ͨ��1���ݻ����� */
extern uint16_t StoreBuf[4*1024];
//��һ�׶����ݱ��� ���жϴ���ʱ�������ʱ�����ǰ500������ 500�������Ը�
//�ڶ��׶����ݱ���  ���жϴ����� DMA�жϴ��� ��ʾ ��һ��2048����Ĳ������ ��ʱ��������㵽����������
//�����׶����ݱ��� �ڶ���DMA�жϴ��� ����������������������ʾ
void SaveData(uint8_t phase1,uint16_t currentsampletime)
{  
    uint16_t i=0;
    if(phase1 ==1) //�׶�1 ���ݱ���
    {
        if(currentsampletime >NUM_BEFORE_EXT)  //�ж�ʱ �Ѿ��ɼ��� 800 �ȴ� 300-800 500����
        {
         for(i=0;i<NUM_BEFORE_EXT;i++)
            {

                StoreBuf[i] = Ch1Buf[currentsampletime-NUM_BEFORE_EXT+i];  
            }
        }
        
        else  //�ж�ʱ ֻ�ɼ��� 200  ��Ҫ��Ch1Buf ĩβȡ300�� �ټ����Ѿ��ɼ���200�� ��ϳ�500����
        {
          for(i=0;i<NUM_BEFORE_EXT-currentsampletime  ;i++) //�ȴ�300��
            {
               StoreBuf[i] = Ch1Buf[2048-NUM_BEFORE_EXT+currentsampletime+i];  
            }
           for(i=0 ;i<currentsampletime ;i++) //�ٴ�200��-��200�� ��λ��Ch1Buf��ͷ
            {
                StoreBuf[i+NUM_BEFORE_EXT-currentsampletime] = Ch1Buf[i];
            }
        }
       
    }
    else if(phase1 ==2) //�׶�2 ���ݱ��� �����500֮��
    {
        for(i= 0 ;i<2048-currentsampletime;i++)
        {
            StoreBuf[i+NUM_BEFORE_EXT] = Ch1Buf[i+currentsampletime];//��������֮�� ������ʱ�����ݴ�������
        }
    }
    
    else  //�׶�3 ���ݱ���
    {
        for(i=0;i<currentsampletime;i++)
        {
           StoreBuf[i+2048+NUM_BEFORE_EXT-currentsampletime] =  Ch1Buf[i];
            
        }
        Phaseindex =0 ;//�ָ���ʼ״̬
        StopADC();
        WriteExcelTest();
        bsp_StartTimer(0,500);
        bsp_StartAutoTimer(1,100); 
    }
}
 void WriteExcelTest(void)
{
    FRESULT result;
	FATFS fs;
	FIL file;
	//DIR DirInf;
	//uint32_t bw;
	uint32_t i;//,k;
//	uint32_t runtime1,runtime2,timelen;
	//uint8_t err = 0;
	char TestFileName[50];
	static uint8_t s_ucTestSn = 0;
    
    result = f_mount(&fs, FS_VOLUME_SD, 0);	
    sprintf(TestFileName, "myexceltest%04d.xls", ADCSampleTimeSave);
    result = f_open(&file, TestFileName, FA_OPEN_ALWAYS | FA_WRITE);
    for(i=0;i<2048+500;i++)
    {
        f_lseek(&file,f_size(&file));    
        f_printf(&file,"%d\t%04d\r\n",i,StoreBuf[i]);
    }
    /* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	f_mount(NULL,FS_VOLUME_SD, 0);
    //printf("test excel ok\r\n");
    
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
