#include "bsp.h"
#define TFTCOM   COM2
uint8_t tftdata[10]={0xA5,0X5A,0X05,0X82,0X00,0x03,0X10,0x28};  //修改变量0000 的值
uint8_t tftcmd[10]={0xA5,0X5A,0X03,0X80,0X4f,0x03};     //模拟按键控制
uint8_t tftText[30] ={0xA5,0X5A,0X0D,0X82};
uint8_t g_page =0;
void bsp_Diwen_Updatedata(uint16_t dataaddr,uint16_t data_16)
{
 
    tftdata[4] = dataaddr>>8;
    tftdata[5] = dataaddr&0x00ff;
    tftdata[6] = data_16>>8;
    tftdata[7] = data_16&0x00ff;
    
    comSendBuf(COM2,tftdata,8);
} 

void bsp_SendKey(uint8_t key)
{
     tftcmd[4] = 0x4f;
     tftcmd[5] = key;
     comSendBuf(COM2,tftcmd,6);
}
void bsp_EnableTouch(void)
{
     
     tftcmd[4] = 0x0b;
     tftcmd[5] = 0xff;
     comSendBuf(COM2,tftcmd,6);
}
void bsp_DisableTouch(void)
{
     tftcmd[4] = 0x0b;
     tftcmd[5] = 0x00;
     comSendBuf(COM2,tftcmd,6);
}


//切换指定页面
void bsp_ScreenChange(uint8_t _page)
{
     uint8_t pagedata[7] = {0xA5,0X5A,0X04,0X80,0X03,0X00,_page};
     
     comSendBuf(COM2,pagedata,7);
}

void bsp_ScreenUPdateSYSSTATUS(uint8_t sta)
{
    
    tftText[3] = 0x82; //寄存器
    tftText[4] = 0x01; //地址
    tftText[5] = 0x40;
//    if(sta == 0)  //启动阶段
//    {
//        //待机  B4FDBBFA
//    tftText[2] = 0X09; //数据长度   = 总长-3
//			
//    tftText[6] = 0xB4;
//    tftText[7] = 0xFD;
//    
//    tftText[8] = 0xBB;
//    tftText[9] = 0xFA;
//    
//    tftText[10] = 0xFF;
//    tftText[11] = 0xFF;
//    
//          
//    comSendBuf(TFTCOM,tftText,12);
//    }
		
 

//    else if(sta >9 ) //运行结束
//    {
//        //完成 CDEAB3C9
//    tftText[2] = 0X09; //长度
//    tftText[6] = 0XCD;
//    tftText[7] = 0xEA;
//        
//    tftText[8] = 0xB3;
//    tftText[9] = 0xC9;
//    
//    tftText[10] = 0xFF;
//    tftText[11] = 0xFF;

//     comSendBuf(TFTCOM,tftText,12);
//    }
    
    if(sta ==0 ) //停止
    {
        //停止 CDA3D6B9
    tftText[2] = 0X09; //长度
    tftText[6] = 0XCD;
    tftText[7] = 0xA3;
        
    tftText[8] = 0xD6;
    tftText[9] = 0xB9;
    
    tftText[10] = 0xFF;
    tftText[11] = 0xFF;

     comSendBuf(TFTCOM,tftText,12);
    }
    
//   else if((sta ==STATUS_HUMPROTECT1) ||(sta ==STATUS_HUMPROTECT2)) //停止
//    {
//        //保护 B1A3BBA4
//    tftText[2] = 0X09; //长度
//    tftText[6] = 0XB1;
//    tftText[7] = 0xA3;
//        
//    tftText[8] = 0xBB;
//    tftText[9] = 0xA4;
//    
//    tftText[10] = 0xFF;
//    tftText[11] = 0xFF;

//     comSendBuf(TFTCOM,tftText,12);
//    }
    
//  else if(sta ==STATUS_NEEDREPAIR)  //待维修
//    {
//        //待维修 B4FDCEACD0DE
//    tftText[2] = 0X09; //长度
//    tftText[6] = 0XB4;
//    tftText[7] = 0xFD;
//        
//    tftText[8] = 0xCE;
//    tftText[9] = 0xAC;
//    
//    tftText[10] = 0xD0;
//    tftText[11] = 0xDE;
//        
//    tftText[12] = 0xFF;
//    tftText[13] = 0xFF;

//     comSendBuf(TFTCOM,tftText,14);
//    }
    
   else {  //运行

    tftText[2] = 0X09; //长度
			
    tftText[6] = 0XD4;
    tftText[7] = 0xCB;
    
    tftText[8] = 0xD0;
    tftText[9] = 0xD0;
    
    tftText[10] = 0xFF;
    tftText[11] = 0xFF;
    
			
    comSendBuf(TFTCOM,tftText,12);
			
    }
     
}