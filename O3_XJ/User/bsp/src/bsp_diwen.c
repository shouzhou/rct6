#include "bsp.h"

#define TFTCOM   COM4
uint8_t tftText[30] ={0xA5,0X5A,0X0D,0X82};
uint8_t tftdata[10]={0xA5,0X5A,0X05,0X82,0X00,0x03,0X10,0x28};  //�޸ı���0000 ��ֵ
uint8_t tftcmd[10]={0xA5,0X5A,0X03,0X80,0X4f,0x03};     //ģ�ⰴ������
void bsp_ScreenUpdateText(void)
{
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(10);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(10);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(10);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(10);
}
void bsp_ScreenUPdateSYSSTATUS(uint8_t sta)
{
    
    tftText[3] = 0x82; //�Ĵ���
    tftText[4] = 0x01; //��ַ
    tftText[5] = 0x00;
    if(sta == STATUS_SYSINIBEGIN)
    {

 //ϵCFB5ͳCDB3��B3F5ʼCABC��BBAF��D6D0   
    tftText[2] = 0X12; //����    
    tftText[6] = 0xcf;
    tftText[7] = 0xb5;
    
    tftText[8] = 0xcd;
    tftText[9] = 0xb3;
    
    tftText[10] = 0xb3;
    tftText[11] = 0xf5;
    
    tftText[12] = 0xca;
    tftText[13] = 0xbc;
    
    tftText[14] = 0xbb;
    tftText[15] = 0xaf;
    
    tftText[16] = 0xd6;
    tftText[17] = 0xd0;
    
    tftText[18] = 0xff;
    tftText[19] = 0xff;
    
    comSendBuf(TFTCOM,tftText,20);
    }
    else if(sta == SYSINIFINISH){
//        ��B3F5ʼCABC��BBAF��CDEA��B3C9
    tftText[2] = 0X10; //����
    tftText[6] = 0XB3;
    tftText[7] = 0xF5;
    
    tftText[8] = 0xCA;
    tftText[9] = 0xBC;
    
    tftText[10] = 0xBB;
    tftText[11] = 0xAF;
    
    tftText[12] = 0xCD;
    tftText[13] = 0xEA;
    
    tftText[14] = 0xB3;
    tftText[15] = 0xC9;
    tftText[16] = 0xff;
    tftText[17] = 0xff;
        comSendBuf(TFTCOM,tftText,18);
    }
    else if((sta>=STATUS_AUTOV_START)&&(sta<STATUS_AUTOV_RUN_ON))
    {
//        ϵCFB5ͳCDB3��D4CB��D0D0��D6D0
    tftText[2] = 0X10; //����
    tftText[6] = 0XCF;
    tftText[7] = 0xB5;
    
    tftText[8] = 0xCD;
    tftText[9] = 0xB3; 
    
    tftText[10] = 0xD4;
    tftText[11] = 0xCB;
    
    tftText[12] = 0xD0;
    tftText[13] = 0xD0;
    
    tftText[14] = 0xD6;
    tftText[15] = 0xD0;
    tftText[16] = 0xff;
    tftText[17] = 0xff;
        comSendBuf(TFTCOM,tftText,18);
        
    }
    else if((sta ==STATUS_AUTOV_RUN_ON)||(sta == STATUS_AUTOV_RUN_OFF))
    {
        //        ��D4CB//��D0D0//��BDE1//��CAF8
        //��D5F0��B5B4��BDD7��B6CE

        //      ��D4CB��D0D0

        tftText[2] = 0X0A; //����
        tftText[6] = 0XD4;
        tftText[7] = 0xCB;
            
        tftText[8] = 0xD0;
        tftText[9] = 0xD0;

        tftText[10] = 0XFF;
        tftText[11] = 0xFF;
        
        comSendBuf(TFTCOM,tftText,12);
    }
    else if(sta ==STATUS_AUTOV_WAIT ) //���н���
    {
//        ��D4CB//��D0D0//��BDE1//��CAF8
    tftText[2] = 0X0E; //����
    tftText[6] = 0XD4;
    tftText[7] = 0xCB;
        
    tftText[8] = 0xD0;
    tftText[9] = 0xD0;
    
    tftText[10] = 0xBD;
    tftText[11] = 0xE1;
    
    tftText[12] = 0xCA;
    tftText[13] = 0xF8;
    
    tftText[14] = 0xff;
    tftText[15] = 0xff;
        comSendBuf(TFTCOM,tftText,16);
    }
     
}

void bsp_ScreenUpdateNet(uint8_t _sta,uint8_t withoutnet)//onenet״̬��ʾ
{
    tftText[3] = 0x82; //�Ĵ���
    tftText[4] = 0x01; //��ַ
    tftText[5] = 0x50;
   if(withoutnet ==1) //�ر������ϴ�
   {
        //��B9D8��B1D5

        tftText[2] = 0X0a; //����
        tftText[6] = 0Xb9;
        tftText[7] = 0xd8;

        tftText[8] = 0xb1;
        tftText[9] = 0xd5;


        tftText[10] = 0xff;
        tftText[11] = 0xff;
        comSendBuf(TFTCOM,tftText,12);
        
   }
    else
    {
          //   ��BFAA��C6F4
            tftText[2] = 0X0a; //����
            tftText[6] = 0Xbf;
            tftText[7] = 0xaa;

            tftText[8] = 0xc6;
            tftText[9] = 0xf4;


            tftText[10] = 0xff;
            tftText[11] = 0xff;
            comSendBuf(TFTCOM,tftText,12);
       
    }
}
void bsp_ScreenUPdateChemical(uint8_t chemical)
{
    //////////////////////////////////////////////////////   
    if(chemical ==1) //�Ƿ�������
   {
        //       ��CAC7
        tftText[2] = 0X08; //����
        tftText[4] = 0x02; //��ַ
        tftText[5] = 0x50;
       
        tftText[6] = 0Xca;
        tftText[7] = 0xc7;

        tftText[8] = 0xff;
        tftText[9] = 0xff;
        comSendBuf(TFTCOM,tftText,10);
        
   }
   else if(chemical == 0)
   {
       //     ��B7F1
        tftText[2] = 0X08; //����
        tftText[4] = 0x02; //��ַ
        tftText[5] = 0x50;
       
        tftText[6] = 0Xb7;
        tftText[7] = 0xf1;

        tftText[8] = 0xff;
        tftText[9] = 0xff;
        comSendBuf(TFTCOM,tftText,10);
   }
   
}
void bsp_ScreenUPdateType(uint8_t plant)//ֲ����Ϣ��ʾ
{
    tftText[3] = 0x82; //�Ĵ���
    
   if(plant ==0) //����
   {
        //        ��C3E7��CCE5 ��C6DA
        tftText[2] = 0X0a; //����
        tftText[4] = 0x02; //��ַ
        tftText[5] = 0x00;
       
        tftText[6] = 0Xc3;
        tftText[7] = 0xe7;

        tftText[8] = 0xc6;
        tftText[9] = 0xda;


        tftText[10] = 0xff;
        tftText[11] = 0xff;
        comSendBuf(TFTCOM,tftText,12);
        
   }
   else if(plant == 1)
   {
        //      ��B3C9��CAEC��CCE5
        //      ��B3C9��D6EA��C6DA

        tftText[2] = 0X0c; //����
        tftText[4] = 0x02; //��ַ
        tftText[5] = 0x00;
       
        tftText[6] = 0Xb3;
        tftText[7] = 0xc9;

        tftText[8] = 0xd6;
        tftText[9] = 0xea;

        
        tftText[10] = 0xc6;
        tftText[11] = 0xda;
       
        tftText[12] = 0xff;
        tftText[13] = 0xff;
        comSendBuf(TFTCOM,tftText,14);
   }

   
}
void bsp_Diwen_Updatedata(uint16_t dataaddr,uint16_t data_16)
{
 
    tftdata[4] = dataaddr>>8;
    tftdata[5] = dataaddr&0x00ff;
    tftdata[6] = data_16>>8;
    tftdata[7] = data_16&0x00ff;
    
    comSendBuf(TFTCOM,tftdata,8);
} 

void bsp_SendKey(uint8_t key)
{
     tftcmd[4] = 0x4f;
     tftcmd[5] = key;
     comSendBuf(TFTCOM,tftcmd,6);
}

void bsp_EnableTouch(void)
{
     
     tftcmd[4] = 0x0b;
     tftcmd[5] = 0xff;
     comSendBuf(TFTCOM,tftcmd,6);
}
void bsp_DisableTouch(void)
{
     tftcmd[4] = 0x0b;
     tftcmd[5] = 0x00;
     comSendBuf(TFTCOM,tftcmd,6);
}

void bsp_PlayMusic(void)
{
    uint8_t musicdata[10] = {0xA5,0X5A,0X05,0X80,0X50,0X5B,0X00,0X01};
    comSendBuf(TFTCOM,musicdata,8);
}

//�л�ָ��ҳ��
void bsp_ScreenChange(uint8_t _page)
{
     uint8_t pagedata[7] = {0xA5,0X5A,0X04,0X80,0X03,0X00,_page};
     
      comSendBuf(TFTCOM,pagedata,7);
}
