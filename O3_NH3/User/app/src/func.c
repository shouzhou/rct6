#include "bsp.h"
#define LOWMAXTIME    300   //�޸�����꣬�ı��Ũ��ʱʱ��
uint16_t g_mannualtimerun =0,g_loopruntimerun =0 ,g_loopstoptimerun =0,g_nh3runtimerun=0,g_nh3stoptimerun=0;
uint16_t g_Nh3LowEnterCount=0; //��Ũ�Ƚ������ 
void FuncManual(uint16_t mode) //�ֶ�ģʽ
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
    if(mode == STOPAUTODEN) //����ֹͣ
    {
        bsp_IOOn(1);
        bsp_IOOn(2);
        bsp_IOOn(3);
        bsp_IOOn(4);
        bsp_IOOn(5);
    }
    else  //���ݰ���Ũ�Ƚ��п���
    {
        if(g_nh3den >50) //>50ppm -ȫ��
        {
            bsp_IOOff(1); 
            bsp_IOOff(2);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
            g_Nh3LowEnterCount=0;
        }
        else if(g_nh3den >40) //40 -50ppm ������
        {
            bsp_IOOn(1);
            bsp_IOOff(2);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
            g_Nh3LowEnterCount=0;
            
        }
         else if(g_nh3den >10) //10 -40 ppm ������
        {
            bsp_IOOn(2);
            bsp_IOOn(1);
            bsp_IOOff(3);
            bsp_IOOff(4);
            bsp_IOOff(5);
            g_Nh3LowEnterCount=0;
        }
        else if(g_nh3den >=0)//1 -10 ppm ѭ������
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
            else //ֹͣ
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
void FuncLoop(uint16_t mode) //ѭ��ģʽ
{
    if(mode == STOPLOOP) //����ֹͣ
    {
        bsp_IOOn(1);
        bsp_IOOn(2);
        bsp_IOOn(3);
        bsp_IOOn(4);
        bsp_IOOn(5);
    }
    else
    {
        if(g_loopstatus==1) //��
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