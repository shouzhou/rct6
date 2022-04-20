#include "bsp.h"
#define LOWMAXTIME      300   //修改这个宏，改变低浓度时开合关的总时间
#define LOWSWITCHTIME   60    //修改这个宏，改变低浓度时每次开和关的时间
#define FANOFFTIME      120   //修改这个宏，改变风机延迟关闭时间
#define AUTOVONTIME    120     //修改这个宏，改变自动体积模式下震荡开时间
#define AUTOVOFFTIME   120     //修改这个宏，改变自动体积模式下震荡关时间

#define O3LEVEL1        50    //修改这个宏，改变臭氧浓度值1
#define O3LEVEL2        40    //修改这个宏，改变臭氧浓度值2
#define O3LEVEL3        10    //修改这个宏，改变臭氧浓度值3
#define O3LEVEL4        0     //修改这个宏，改变臭氧浓度值4-这个基本不变
//湿度分段
#define HLEVEL1    950 
#define HLEVEL2    850 
#define HLEVEL3    800 
#define HLEVEL4    750 
#define HLEVEL5    650 
#define HLEVEL6    600 
#define HLEVEL7    550 
#define HLEVEL8    550 
//温度分段
#define TLEVEL1    600 
#define TLEVEL2    550 
#define TLEVEL3    500 
#define TLEVEL4    450 
#define TLEVEL5    400 
#define TLEVEL6    300 
#define TLEVEL7    250 
#define TLEVEL8    200 
#define TLEVEL9    100 



uint8_t g_status =0;   //系统运行的所有状态切换
uint16_t g_timerun=0;
uint16_t g_Nh3LowEnterCount=0; //低浓度进入次数 
uint16_t g_AutoVEntercount=0; //体积模式下震荡阶段计数变量
uint8_t g_o3SwitchIndex=1;     //切换序号
uint16_t g_V=1;
float g_AutoVtime =10.0;
float kv = 1.0;
float kc = 1.0;
extern DHT11_T  g_DHT11;
float FunO3CalParameter(void)
{
    float kh = 1.0;
    float kt = 1.0;
    ////湿度参数计算
    if(g_DHT11.Hum>=HLEVEL1)
    {
        kh = 0.3;
    }
    else if(g_DHT11.Hum>=HLEVEL2)
    {
        kh = 0.35;
    }
    else if(g_DHT11.Hum>=HLEVEL3)
    {
        kh = 0.4;
    }
    else if(g_DHT11.Hum>=HLEVEL4)
    {
        kh = 0.5;
    }
    else if(g_DHT11.Hum>=HLEVEL5)
    {
        kh = 0.6;
    }
    else if(g_DHT11.Hum>=HLEVEL6)
    {
        kh = 0.8;
    }
    else if(g_DHT11.Hum>=HLEVEL7)
    {
        kh = 0.9;
    }
    else if(g_DHT11.Hum>=HLEVEL8)
    {
        kh = 1;
    }
    else
    {
        kh = 1;
    }
    
    ////温度参数 计算
    if(g_DHT11.Temp>=TLEVEL1)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL2)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL3)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL4)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL5)//40
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL6)//30
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL7)//25
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL8)//20
    {
        kt = 0.98;
    }
    else if(g_DHT11.Temp>=TLEVEL9)//10
    {
        kt = 1;
    }
    else
    {
        kt = 1;
    }
    return kh*kt;
}
void FunFanOpen(void)
{
    bsp_IOOff(5);
}
void FunFanClose(void)
{
    bsp_IOOn(5);
}
// onnum 开的个数
// index  从第几个开，暂定一个小时开始切换顺序  1-4
// 
void FunO3Control(uint8_t onnum,uint8_t index)
{
    uint8_t offnum = 4-onnum; //关闭个数
    uint8_t i,j;
    for(i=index;i<onnum+index;i++) //开启部分
    {
        if(i>4)
            bsp_IOOff(i-4);
        else
            bsp_IOOff(i);
    }
    for(j=i;j<offnum+i;j++)
    {
        if(j>4)
            bsp_IOOn(j-4);
        else
            bsp_IOOn(j);
    }
   
}
extern uint16_t g_nh3den;
void FunCheckStatus(void)  //自动浓度模式下的状态切换
{
    
        if(g_nh3den >O3LEVEL1) //>50ppm -全开
        {
             g_status = STATUS_AUTODEN_RUN_F1;             
        }
        else if(g_nh3den >O3LEVEL2) //40 -50ppm 开三个
        {
             g_status = STATUS_AUTODEN_RUN_F2; 
        }
        else if(g_nh3den >O3LEVEL3) //10 -40 ppm 开两个
        {
             g_status = STATUS_AUTODEN_RUN_F3; 
        }
        else if(g_nh3den >=O3LEVEL4)//1 -10 ppm 循环开关
        {
            g_status = STATUS_AUTODEN_RUN_F4; 
        }
        g_Nh3LowEnterCount =0;
        g_timerun =0;
 }                  
void FunSwitch(void)
{
        switch(g_status)
        {
            case STATUS_ALL_STOP :
              //确保所有模块都是关闭的
            FunO3Control(0,g_o3SwitchIndex);
            FunFanClose();
            // 根据选择进入对应的不同模式
               if(g_func == STARTMANUAL) //手工模式
               {
                  g_status = STATUS_MANUAL_START; //进入手工模式
               }
               else if(g_func == STARTLOOP) //循环模式
               {
                  g_status = STATUS_LOOP_START; //进入循环模式
               }
               else if(g_func == STARTAUTODEN) //自动浓度模式
               {
                  g_status = STATUS_AUTODEN_START; //进入自动浓度模式
               }
               else if(g_func == STARTAUTOV) //自动体积模式
               {
                  g_status = STATUS_AUTOV_START; //进入自动体积模式
               }
               
              break;
//---------------------------手工模式 状态轮询------------------begin----------------------------//               
            case STATUS_MANUAL_START: //进入手工模式 ，开启继电器，只开一次
                FunO3Control(4,g_o3SwitchIndex);
                FunFanOpen();
                g_status = STATUS_MANUAL_RUN_D1; //进入手工模式延时阶段
              
                
                break;
            case STATUS_MANUAL_RUN_D1: 
                g_timerun++;
                if(g_func ==STOPMANUAL) // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                {
                   g_timerun =0;
                   g_status = STATUS_MANUAL_STOP_D1;
                   FunO3Control(0,g_o3SwitchIndex);
                }
               if(g_timerun>g_mannualtime/10*60)
               {
                   g_status = STATUS_MANUAL_STOP_D1; //进入停止前等待模式
                   g_timerun =0 ; //计时值清零
                   // 等待时间到 关闭臭氧 保持风扇开启
                   FunO3Control(0,g_o3SwitchIndex);
               }
                break;
            case STATUS_MANUAL_STOP_D1:
                g_timerun++;
               if(g_timerun>FANOFFTIME) //等待300S后 所有关闭
               {
                   g_status = STATUS_ALL_STOP; //进入停止前等待模式
                   g_timerun =0 ; //计时值清零
                   
               }
                break;
//---------------------------手工模式 状态轮询------------------end----------------------------// 
               
               

//---------------------------循环模式 状态轮询------------------begin----------------------------//                 

            case  STATUS_LOOP_START :
                FunO3Control(4,g_o3SwitchIndex);
                FunFanOpen();
                g_status = STATUS_LOOP_RUN_F1; //进入循环模式
                break;
            case STATUS_LOOP_RUN_F1: //循环中的启动阶段
                g_timerun++;
               if(g_func ==STOPLOOP) // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                {
                   g_timerun =0;
                   g_status = STATUS_LOOP_STOP_D1;
                   FunO3Control(0,g_o3SwitchIndex);
                }
               if(g_timerun>g_loopruntime /10 * 60) //等待设置S后 进入循环停止阶段
               {
                   g_status = STATUS_LOOP_RUN_F2; //进入循环中的停止阶段
                   g_timerun =0 ; //计时值清零
                   FunO3Control(0,g_o3SwitchIndex);
                   
               }
                break;
            case STATUS_LOOP_RUN_F2: //循环中的停止阶段
                g_timerun++;
                if(g_func ==STOPLOOP) // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                {
                   g_timerun =0;
                   g_status = STATUS_LOOP_STOP_D1;
                   FunO3Control(0,g_o3SwitchIndex);
                }
               if(g_timerun>g_loopstoptime /10 * 60) //等待设置S后 进入循环停止阶段
               {
                   g_status = STATUS_LOOP_RUN_F1; //进入循环中的开启阶段
                   g_timerun =0 ; //计时值清零
                   FunO3Control(4,g_o3SwitchIndex);
                   
               }
                break;
               
              case STATUS_LOOP_STOP_D1: //延时结束模式
               g_timerun++;
               if(g_timerun>FANOFFTIME) //等待300S后 所有关闭
               {
                   g_status = STATUS_ALL_STOP; //进入停止前等待模式
                   g_timerun =0 ; //计时值清零
                   
               }
                break;
            
            
            
//---------------------------循环模式 状态轮询------------------end----------------------------//     

    
               

//---------------------------自动浓度模式 状态轮询------------------begin----------------------------// 
               case STATUS_AUTODEN_START: //自动浓度模式开始
                FunO3Control(4,g_o3SwitchIndex);
                FunFanOpen();
                FunCheckStatus();

                break;
               case STATUS_AUTODEN_RUN_F1://>50
                  FunO3Control(4,g_o3SwitchIndex);  
                 if(g_nh3den<=O3LEVEL1) FunCheckStatus();
                 if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
               case STATUS_AUTODEN_RUN_F2://>40-50
                 FunO3Control(3,g_o3SwitchIndex); 
                 if((g_nh3den>O3LEVEL1)||(g_nh3den<=O3LEVEL2)) FunCheckStatus();
                 if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
               case STATUS_AUTODEN_RUN_F3://>10-40
                 FunO3Control(2,g_o3SwitchIndex);
                if((g_nh3den>O3LEVEL2)||(g_nh3den<=O3LEVEL3)) FunCheckStatus();
                if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
               case STATUS_AUTODEN_RUN_F4: //低浓度模式 先只开一个 然后进入开关循环交替模式
                   FunO3Control(1,g_o3SwitchIndex);
                   g_status = STATUS_AUTODEN_RUN_F4_ON;
                   break;
               case STATUS_AUTODEN_RUN_F4_ON://>=0
                   if((g_nh3den>O3LEVEL4)) FunCheckStatus();
                   g_timerun++;
                   g_Nh3LowEnterCount++;
                  if(g_timerun>LOWSWITCHTIME) //开始进入低浓度off模式
                  {
                    FunO3Control(0,g_o3SwitchIndex);
                    g_timerun =0;
                    g_status = STATUS_AUTODEN_RUN_F4_OFF;
                      
                  }
                  if(g_Nh3LowEnterCount>LOWMAXTIME) //进入待机等待
                  {
                    g_status =  STATUS_AUTODEN_WAIT_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                  }
                  if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
                  
                case STATUS_AUTODEN_RUN_F4_OFF://>=0
                   if((g_nh3den>O3LEVEL4)) FunCheckStatus();
                   g_timerun++;
                   g_Nh3LowEnterCount++;
                  if(g_timerun>LOWSWITCHTIME) //开始进入低浓度on模式
                  {
                    FunO3Control(1,g_o3SwitchIndex);
                    g_timerun =0;
                    g_status = STATUS_AUTODEN_RUN_F4_ON;
                      
                  }
                  if(g_Nh3LowEnterCount>LOWMAXTIME) //进入待机等待
                  {
                    g_status =  STATUS_AUTODEN_WAIT_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                  }
                  if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
                  
                case STATUS_AUTODEN_STOP_D1:
                g_timerun++;
               if(g_timerun>FANOFFTIME) //等待300S后 所有关闭
               {
                   g_status = STATUS_ALL_STOP; //进入关闭模式
                   g_timerun =0 ; //计时值清零
                   FunFanClose(); //关闭风机
               }
                    break;
                case STATUS_AUTODEN_WAIT_D1: //待机等待
                    g_timerun++;
                   if(g_timerun>FANOFFTIME) //等待300S后 所有关闭
                   {
                       g_status = STATUS_AUTODEN_WAIT; //进入待机模式
                       g_timerun =0 ; //计时值清零
                       FunFanClose(); //关闭风机
                   }
                    if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                     {
                        g_status =  STATUS_AUTODEN_STOP_D1;
                        g_Nh3LowEnterCount =0;
                        g_timerun =0;
                        FunO3Control(0,g_o3SwitchIndex);
                     }
                    break;
                
                case STATUS_AUTODEN_WAIT : //待机模式
                    if(g_nh3den>10)   //浓度升高时开始切换
                    {
                        FunCheckStatus();
                        FunFanOpen();
                    }
                    if(g_func == STOPAUTODEN)  // 在运行过程中 点击了 关闭按钮 直接跳转到延时结束模式
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                   break;

//---------------------------自动浓度模式 状态轮询------------------end----------------------------//   
    

    
               

//---------------------------自动体积模式 状态轮询------------------begin----------------------------//  
                case STATUS_AUTOV_START:
                    FunO3Control(4,g_o3SwitchIndex);
                    FunFanOpen();
                    g_status = STATUS_AUTOV_RUN_F1;
                    break;
                case STATUS_AUTOV_RUN_F1://2分钟
                   g_timerun++;
                   if(g_timerun>120)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_F2;
                   }
                    break;
                case STATUS_AUTOV_RUN_F2:
                    g_AutoVtime = kv*g_lenth*g_width*g_height-2*FunO3CalParameter();
                    g_status = STATUS_AUTOV_RUN_F3;
                    break;
                case STATUS_AUTOV_RUN_F3: //循环计算时间
                    g_AutoVtime = g_AutoVtime - 2*FunO3CalParameter();
                    if(g_AutoVtime<5)
                    {
                       g_status = STATUS_AUTOV_RUN_ON; //进入震荡模式
                       FunO3Control(2,g_o3SwitchIndex);//开2个
                    }
                    else
                    {
                       g_status = STATUS_AUTOV_RUN_F4; //等待2分钟
                    }
                    break;
                case STATUS_AUTOV_RUN_F4:
                    g_timerun++;
                   if(g_timerun>120)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_F3; //继续回去计算时间
                   }
                    break;
                case STATUS_AUTOV_RUN_ON://震荡模式 开阶段
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>AUTOVONTIME)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_OFF; //进入震荡 关阶段
                      FunO3Control(0,g_o3SwitchIndex);
                   }
                   if(g_AutoVEntercount>kc*g_lenth*g_width*g_height)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       g_status = STATUS_AUTOV_WAIT_D1;
                   }
                    break;
                case STATUS_AUTOV_RUN_OFF://震荡模式 开阶段
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>AUTOVOFFTIME)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_ON; //进入震荡 开阶段
                      FunO3Control(2,g_o3SwitchIndex);
                       FunO3Control(0,1);
                   }
                    if(g_AutoVEntercount>kc*g_lenth*g_width*g_height)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       g_status = STATUS_AUTOV_WAIT_D1;
                       FunO3Control(0,1);
                   }
                    break;
                 case STATUS_AUTOV_WAIT_D1://等待待机
                     g_timerun++;
                     if(g_timerun>300)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_WAIT; //进入震荡 开阶段
                     
                       FunFanClose();
                   }
                   
                    break;
                  case STATUS_AUTOV_WAIT://待机中
                     g_timerun++;
                                        
                    break;
                
                 
//---------------------------自动体积模式 状态轮询------------------end----------------------------//    
            default:
                break;
            
        }
        g_func=0xff;  //g_func 恢复初始值
}
//-----------------------FunSwitch--------状态切换 end-------------------------//
