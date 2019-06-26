#include "bsp.h"

uint8_t tftdata[10]={0xA5,0X5A,0X05,0X82,0X00,0x03,0X10,0x28};  //修改变量0000 的值
uint8_t tftcmd[10]={0xA5,0X5A,0X03,0X80,0X4f,0x03};     //模拟按键控制
void bsp_Diwen_Updatedata(uint16_t dataaddr,uint16_t data_16)
{
    //  comSendBuf(COM1,tftdata,8);     
    //  comSendBuf(COM1,tftcmd,6);   
    tftdata[4] = dataaddr>>8;
    tftdata[5] = dataaddr&0x00ff;
    tftdata[6] = data_16>>8;
    tftdata[7] = data_16&0x00ff;
    
    comSendBuf(COM2,tftdata,8);
} 

void bsp_SendKey(uint8_t key)
{
    tftcmd[5] = key;
   // USART1_Write(USART1,tftcmd,6);
     comSendBuf(COM2,tftcmd,6);
}

