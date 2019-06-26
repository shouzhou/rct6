/*
*********************************************************************************************************
*
*	模块名称 : SD卡Fat文件系统演示模块。
*	文件名称 : demo_sdio_fatfs.c
*	版    本 : V1.0
*	说    明 : 该例程移植FatFS文件系统（版本 R0.09b），演示如何创建文件、读取文件、创建目录和删除文件
*			并测试了文件读写速度。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "ff.h"			/* FatFS文件系统模块*/
#include "demo_sd_fatfs.h"

#define  NUM_BEFORE_EXT  500         //预定义 在触发前需要保留的数据个数
extern uint16_t Ch1Buf[2048];	 /* 通道1数据缓冲区 */
extern uint16_t StoreBuf[4*1024];
//第一阶段数据保存 当中断触发时，保存改时间点以前500个数据 500个数可以改
//第二阶段数据保存  当中断触发后 DMA中断触发 表示 第一次2048个点的采样完成 此时保存采样点到结束的数据
//第三阶段数据保存 第二次DMA中断触发 保存后续的数据用于输出显示
void SaveData(uint8_t phase1,uint16_t currentsampletime)
{  
    uint16_t i=0;
    if(phase1 ==1) //阶段1 数据保存
    {
        if(currentsampletime >NUM_BEFORE_EXT)  //中断时 已经采集了 800 先存 300-800 500个数
        {
         for(i=0;i<NUM_BEFORE_EXT;i++)
            {

                StoreBuf[i] = Ch1Buf[currentsampletime-NUM_BEFORE_EXT+i];  
            }
        }
        
        else  //中断时 只采集到 200  需要到Ch1Buf 末尾取300个 再加上已经采集的200个 组合成500个数
        {
          for(i=0;i<NUM_BEFORE_EXT-currentsampletime  ;i++) //先存300个
            {
               StoreBuf[i] = Ch1Buf[2048-NUM_BEFORE_EXT+currentsampletime+i];  
            }
           for(i=0 ;i<currentsampletime ;i++) //再存200个-该200个 是位与Ch1Buf开头
            {
                StoreBuf[i+NUM_BEFORE_EXT-currentsampletime] = Ch1Buf[i];
            }
        }
       
    }
    else if(phase1 ==2) //阶段2 数据保存 序号在500之后
    {
        for(i= 0 ;i<2048-currentsampletime;i++)
        {
            StoreBuf[i+NUM_BEFORE_EXT] = Ch1Buf[i+currentsampletime];//将采样点之后 到结束时的数据存入数组
        }
    }
    
    else  //阶段3 数据保存
    {
        for(i=0;i<currentsampletime;i++)
        {
           StoreBuf[i+2048+NUM_BEFORE_EXT-currentsampletime] =  Ch1Buf[i];
            
        }
        Phaseindex =0 ;//恢复初始状态
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
    /* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL,FS_VOLUME_SD, 0);
    //printf("test excel ok\r\n");
    
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
