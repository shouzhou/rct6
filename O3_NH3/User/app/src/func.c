#include "bsp.h"
#define LOWMAXTIME    300   //修改这个宏，改变低浓度时时间
uint16_t g_mannualtimerun =0,g_loopruntimerun =0 ,g_loopstoptimerun =0,g_nh3runtimerun=0,g_nh3stoptimerun=0;
uint16_t g_Nh3LowEnterCount=0; //低浓度进入次数 
void FuncManual(uint16_t mode) //手动模式
{
    if(mode == STOPMANUAL)
    {
        bsp_IOOn(1);
        bsp_IOOn(2);
        bsp_IOOn(3);
        bsp_IOOn(4);
        bsp_IOOn(5);
    }
    else
    {
        if(g_mannualtimerun<g_mannualtime/10*60)
        {
            bsp_IOOff(1); 
            bsp_IOOff(2);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
        }
        else
        {
            bsp_IOOn(1);
            bsp_IOOn(2);
            bsp_IOOn(3);
            bsp_IOOn(4);
            bsp_IOOn(5);
        }
    }    
}
void FuncAutoV(uint16_t mode)
{
    
}
extern uint16_t g_nh3den;
void FuncAutoNH3(uint16_t mode)
{
    if(mode == STOPAUTODEN) //彻底停止
    {
        bsp_IOOn(1);
        bsp_IOOn(2);
        bsp_IOOn(3);
        bsp_IOOn(4);
        bsp_IOOn(5);
    }
    else  //根据氨气浓度进行控制
    {
        if(g_nh3den >50) //>50ppm -全开
        {
            bsp_IOOff(1); 
            bsp_IOOff(2);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
            g_Nh3LowEnterCount=0;
        }
        else if(g_nh3den >40) //40 -50ppm 开三个
        {
            bsp_IOOn(1);
            bsp_IOOff(2);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
            g_Nh3LowEnterCount=0;
            
        }
         else if(g_nh3den >10) //10 -40 ppm 开两个
        {
            bsp_IOOn(2);
            bsp_IOOn(1);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
            g_Nh3LowEnterCount=0;
        }
        else if(g_nh3den >=0)//1 -10 ppm 循环开关
        {
            g_Nh3LowEnterCount++;
            if(g_Nh3LowEnterCount <LOWMAXTIME) //
            {
                if(g_AutoNh3Status ==1)
                {
                    bsp_IOOn(2);
                    bsp_IOOn(1);
                    bsp_IOOn(3);
                    bsp_IOOff(4);
                }
                else
                {
                    bsp_IOOn(2);
                    bsp_IOOn(1);
                    bsp_IOOn(3);
                    bsp_IOOn(4);
                }
            }
            else //停止
            {
                    bsp_IOOn(2);
                    bsp_IOOn(1);
                    bsp_IOOn(3);
                    bsp_IOOn(4);
                    bsp_IOOn(5);
                
                if(g_Nh3LowEnterCount>30000) g_Nh3LowEnterCount =LOWMAXTIME+1;
            }
        }
        
        
    }
}
void FuncLoop(uint16_t mode) //循环模式
{
    if(mode == STOPLOOP) //彻底停止
    {
        bsp_IOOn(1);
        bsp_IOOn(2);
        bsp_IOOn(3);
        bsp_IOOn(4);
        bsp_IOOn(5);
    }
    else
    {
        if(g_loopstatus==1) //开
        {
            bsp_IOOff(1); 
            bsp_IOOff(2);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
        }
        else
        {
            bsp_IOOn(1);
            bsp_IOOn(2);
            bsp_IOOn(3);
            bsp_IOOn(4);
            bsp_IOOn(5);
        }
    }    
}