/********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include <stdio.h>
#include "PLC_Dialogue.h"
#include "PLC_CONF.H"
extern u16 prog_write_buffer[2050];
extern u16 PLC_16BIT[12100];
extern const u16 *PLC_Addr; //地址查表指针
extern const u8  PLC_ROM[]; //PLC程序储存块
/***********************************************
  函数功能：PLC通信错误处理程序
  D8063,M8063=链接,通信错误
  0000=无异常;
  6301=奇偶检验错误
  6302=通信字符错误
  6303=通信和校验出错
  6304=数据格式错误
  6305=指令错误
***********************************************/
void PLC_COMM_ERROR(u16 err_id)
{
    if(err_id!=0)PLC_BIT_ON(M8063);//出错标志 
	D8063=err_id;
}


void write_data(u16 number)
{ 
	u16 temp,appoint_address;
	if(number<12)			                                                   //写入参数的块必需10块
	{
		FLASH_Unlock();		                                                   //flash关闭保护
		FLASH_ErasePage(PLC_RAM_ADDR+number*0x800);                   //erase page 一块数据占用2K
		for(temp=0;temp<1024;temp++)	                                     //操作为16bit,只需要1024次程序操作完毕
		{
			appoint_address=PLC_RAM_ADDR+number*0x800+temp*2;          //起始地址加上块地址再加上块的小地址,等于目标位置 
			FLASH_ProgramHalfWord(appoint_address,prog_write_buffer[temp]);   //等待程序写入结束
		}
		FLASH_Lock();	                                                     //结束程序写入开启flash保护
	}
}

void Read_data(u16 number)		                                           //程序块备份,目的在写程序之前进前面程序备份
{ 
	u16 temp,appoint_address;
	if(number<12)
	{
		for(temp=0;temp<1024;temp++)
		{
			appoint_address=number*0x800+temp*2;                                                  //起始地址加上块地址再加上块的小地址 
			prog_write_buffer[temp]=PLC_ROM[appoint_address]+(PLC_ROM[appoint_address+1]*0x100);	//将程序备份出来
		}
	}
}
/***********************************************
  函数功能：断电数据备份
***********************************************/
void PLC_DATA_KEEP(void)                                                  
{  
	u16 temp=0,backup_addr; 
	for(backup_addr=(0x1000+500);backup_addr<(0x1000+950);backup_addr++) // 备份 400 D 单元到 D500
	prog_write_buffer[temp]=PLC_16BIT[backup_addr],temp++;

	for(backup_addr=(0x0500+100);backup_addr<(0x0500+150);backup_addr++) // 备份 50 T  单元到 T100
	prog_write_buffer[temp]=PLC_16BIT[backup_addr],temp++;

	for(backup_addr=(0x0800+100);backup_addr<(0x0800+150);backup_addr++) // 备份 50 D Unit from C100
	prog_write_buffer[temp]=PLC_16BIT[backup_addr],temp++;

	for(backup_addr=(0x0020);backup_addr<(0x0020+32);backup_addr++)	    // 备份 512-M1023
	prog_write_buffer[temp]=PLC_16BIT[backup_addr],temp++;

	write_data(10);
}

/***********************************************
  函数功能：上电数据复位
***********************************************/
void Recover_data(void)                                                
{ 
	u16 temp=0,backup_addr;
	Read_data(10);

	for(backup_addr=(0x1000+500);backup_addr<(0x1000+950);backup_addr++)
	PLC_16BIT[backup_addr]=prog_write_buffer[temp],temp++;

	for(backup_addr=(0x0500+100);backup_addr<(0x0500+150);backup_addr++)
	PLC_16BIT[backup_addr]=prog_write_buffer[temp],temp++;

	for(backup_addr=(0x0800+100);backup_addr<(0x0800+150);backup_addr++)
	PLC_16BIT[backup_addr]=prog_write_buffer[temp],temp++;

	for(backup_addr=(0x0020);backup_addr<(0x0020+32);backup_addr++)
	PLC_16BIT[backup_addr]=prog_write_buffer[temp],temp++;
}

void RST_T_D_C_M_data(void)		//ADD  小小晟  20151214
{ 
	u16 backup_addr;
	for(backup_addr=(0x1000+0);backup_addr<(0x1000+8000);backup_addr++)		 //D0000-D8000  0X1000--0X2F40
	PLC_16BIT[backup_addr]=0;
	for(backup_addr=(0x0500+0);backup_addr<(0x0500+256);backup_addr++)		 //C0-C256
	PLC_16BIT[backup_addr]=0;
	for(backup_addr=(0x0800+0);backup_addr<(0x0800+256);backup_addr++)		 //T0-T256
	PLC_16BIT[backup_addr]=0;
	for(backup_addr=(0x0000);backup_addr<(0x0020+96);backup_addr++)	         //M0-M3096
	PLC_16BIT[backup_addr]=0;
}




