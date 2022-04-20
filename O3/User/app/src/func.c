#include "bsp.h"

uint16_t g_timerun=0;
//uint16_t g_Nh3LowEnterCount=0; //低浓度进入次数 
uint16_t g_AutoVEntercount=0; //体积模式下震荡阶段计数变量
uint8_t g_o3SwitchIndex=1;     //切换序号
//uint16_t g_V=1;
float g_AutoVtime =10.0;     //全开运行时间
float g_AutoOsctime = 10.0 ; //震荡计算时间
float g_V=0.0;    //全局体积值；
extern DHT11_T  g_DHT11;

float FunO3CalParameterKV(float  _allV)
{
    if(_allV>=KVLEVEL1)
        kv = KVL1;
    else if(_allV>=KVLEVEL2)
        kv = KVL2;
    else if(_allV>=KVLEVEL3)
        kv = KVL3;
    else if(_allV>=KVLEVEL4)
        kv = KVL4;
    else if(_allV>=KVLEVEL5)
        kv = KVL5;
    else if(_allV>=KVLEVEL6)
        kv = KVL6;
    else if(_allV>=KVLEVEL7)
        kv = KVL7;
    else if(_allV>=KVLEVEL8)
        kv = KVL8;
    else 
        kv = KVOTHER;
    
}
float FunO3CalParameter(void)
{
    float kh = 1.0;
    float kt = 1.0;
    ////湿度参数计算
    if(g_DHT11.Hum>=HLEVEL1)
    {
        kh = KHL1;
    }
    else if(g_DHT11.Hum>=HLEVEL2)
    {
        kh = KHL2;
    }
    else if(g_DHT11.Hum>=HLEVEL3)
    {
        kh = KHL3;
    }
    else if(g_DHT11.Hum>=HLEVEL4)
    {
        kh = KHL4;
    }
    else if(g_DHT11.Hum>=HLEVEL5)
    {
        kh = KHL5;
    }
    else if(g_DHT11.Hum>=HLEVEL6)
    {
        kh = KHL6;
    }
    else if(g_DHT11.Hum>=HLEVEL7)
    {
        kh = KHL7;
    }
    else if(g_DHT11.Hum>=HLEVEL8)
    {
        kh = KHL8;
    }
    else
    {
        kh = KHOTHER;
    }
    
    ////温度参数 计算
    if(g_DHT11.Temp>=TLEVEL1)
    {
        kt = KTL1;
    }
    else if(g_DHT11.Temp>=TLEVEL2)
    {
        kt = KTL2;
    }
    else if(g_DHT11.Temp>=TLEVEL3)
    {
        kt = KTL3;
    }
    else if(g_DHT11.Temp>=TLEVEL4)
    {
        kt = KTL4;
    }
    else if(g_DHT11.Temp>=TLEVEL5)//40
    {
        kt = KTL5;
    }
    else if(g_DHT11.Temp>=TLEVEL6)//30
    {
        kt = KTL6;
    }
    else if(g_DHT11.Temp>=TLEVEL7)//25
    {
        kt = KTL7;
    }
    else if(g_DHT11.Temp>=TLEVEL8)//20
    {
        kt = KTL8;
    }
    else if(g_DHT11.Temp>=TLEVEL9)//10
    {
        kt = KTL9;
    }
    else if(g_DHT11.Temp>=TLEVEL10)//5
    {
        kt = KTL10;
    }
    else
    {
        kt = KTOTHER;
    }
    
    if(g_chemical)
    {
        
    }
    else
    {
        
    }
    if(g_plant)
    {
        
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

void FunSwitch(void)
{
    #ifdef USEDEBUG
    if(g_status != 26)
    printf("current status = %d\r\n",g_status);
    
    
    #endif
    if((g_status>10)&&(g_status<STATUS_AUTOV_WAIT))
        g_sysruntime ++;
    
        switch(g_status)
        {
                case STATUS_ALL_STOP :
                  //确保所有模块都是关闭的
                FunO3Control(0,1);
                FunFanClose();
                
                   
                  break;
//                case STATUS_NETINIEND: //网络初始化完成，等待按键实现启动
//                    FunO3Control(0,1);
//                    FunFanClose();
//                    // 根据选择进入对应的不同模式
//                 if(g_funcKey == STARTAUTOV) //自动体积模式
//                   {
//                      g_status = STATUS_AUTOV_START; //进入自动体积模式
//                   }
//                    break;
                   
                case STATUS_AUTOV_START:
                    FunO3Control(0,1);//1-4关
                    FunFanOpen(); //风扇开
                    g_status = STATUS_CHECK_F1_1;
                    g_touch = 11;  //更新显示屏  开始运行
                    TIM_Cmd(TIM6,DISABLE ); //关闭触摸和按键功能
                    break;
                case STATUS_CHECK_F1_1://2分钟 固定时间 不修改了
                    g_timerun++;
                    if(g_timerun>120)
                    {
                        g_status = STATUS_CHECK_F1_2;
                        g_timerun = 0;
                        FunFanClose();
                    }
                    break;
                    case STATUS_CHECK_F1_2://按开机后待机的时间
                    g_timerun++;
                    if(g_timerun>CHECKSTARTTIME) //半小时 自己修改时间  
                    {
                        g_status = STATUS_CHECK_F2;
                        g_timerun = 0;
                        FunFanOpen();
                    }
                    break;
                case STATUS_CHECK_F2:
                    DHT11_ReadData(&g_DHT11);
                    if(g_DHT11.Hum>=CHECKL1HUM) //>=85%
                    {
                        FunO3Control(4,1);//1-4开
                       // FunFanClose(); //风扇关
                        g_status = STATUS_CHECK_F3_L1; 
                    }
                    else if(g_DHT11.Hum>CHECKL2HUM)//>=65% <=85%
                    {
                        FunO3Control(4,1);//1-4开
                       // FunFanClose(); //风扇关
                        g_status = STATUS_CHECK_F3_L2; 
                    }
                    else if(g_DHT11.Hum>CHECKL3HUM)//>=55% <= 65%
                    {
                        FunO3Control(4,1);//1-4开
                       // FunFanClose(); //风扇关
                        g_status = STATUS_CHECK_F3_L3; 
                    }
                    else if(g_DHT11.Hum>=CHECKL4HUM) //>=50% <= 55%
                    {
                        FunO3Control(4,1);//1-4开
                       // FunFanClose(); //风扇关
                        g_status = STATUS_CHECK_F3_L4;
                    }
                    else // <50%  -报警 停止运行
                    {
                        g_status = STATUS_CHECK_F3_L5;
                    }
                    break;
                case STATUS_CHECK_F3_L1://最高湿度延时
                    g_timerun++;
                    if(g_timerun>CHECKL1TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                
                    break;
                case STATUS_CHECK_F3_L2://次高湿度延时
                    g_timerun++;
                    if(g_timerun>CHECKL2TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F3_L3://第三湿度延时
                    g_timerun++;
                    if(g_timerun>CHECKL3TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F3_L4://第四湿度延时
                    g_timerun++;
                    if(g_timerun>CHECKL4TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F3_L5://停机等待湿度变可以运行
                    g_timerun++;
                    if(g_timerun>5)
                    {
                         DHT11_ReadData(&g_DHT11);
                         if(g_DHT11.Hum>=CHECKL4HUM) //>=50%
                        {
                            FunO3Control(4,1);//1-4开
                           // FunFanClose(); //风扇关
                            g_status = STATUS_CHECK_F3_L4;
                        }
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F4://
                     g_status = STATUS_AUTOV_RUN_F1;
//                    g_timerun++;
//                    tim_print_result();
//                    if(g_timerun>=5)
//                    {
//                        g_timerun = 0;
//                        if((g_checkfreok[0]>=3)&&(g_checkfreok[1]>=3)&&(g_checkfreok[2]>=3)&&(g_checkfreok[3]>=3))
//                        {
//                            g_status = STATUS_AUTOV_RUN_F1;
//                            g_checkfreok[0] =0;
//                            g_checkfreok[1] =0;
//                            g_checkfreok[2] =0;
//                            g_checkfreok[3] =0;
//                            TIM_Cmd(TIM3,DISABLE ); //关闭测频功能
//                            bsp_ScreenUPdateSYSSTATUS(g_status);
//                            bsp_DelayMS(100);
//                            bsp_ScreenUPdateSYSSTATUS(g_status);
//                        }
//                        else
//                        {
//                             g_status = STATUS_CHECK_FAIL;
//                        }
//                        
//                    }
                    
                    break;
                case STATUS_CHECK_FAIL :
                    
                    break;
 //--------------------------自动体积模式 状态轮询------------------begin----------------------------//  
                case STATUS_AUTOV_RUN_F1://2分钟
                    g_status = STATUS_AUTOV_RUN_F2;  //这里锁死了  STATUS_AUTOV_RUN_F2  变成了 STATUS_AUTOV_RUN_F1
                    TIM_Cmd(TIM3,DISABLE ); //关闭测频功能
                    
//                   g_timerun++;
//                   if(g_timerun>120)
//                   {
//                      g_timerun =0;
//                      g_status = STATUS_AUTOV_RUN_F2;
//                   }
                    break;
                case STATUS_AUTOV_RUN_F2: //第一次计算
                    kp = FunO3CalParameter();  //0.4
                    g_V = g_lenth*g_width*g_height/1000; //200
                    kv= FunO3CalParameterKV(g_V);  //kv  1.4
                    g_AutoVtime = kv*g_lenth*g_width*g_height/1000.0-60*kp;  //  最后运行时间大概是  第一次g_AutoVtime /24
                    g_status = STATUS_AUTOV_RUN_F3;
                    break;
                case STATUS_AUTOV_RUN_F3: //根据计算时间，判断下一步干什么
                   
                    if(g_AutoVtime<AUTOVCALTIME)
                    {
                       g_status = STATUS_AUTOV_RUN_ON; //进入震荡模式
                       FunO3Control(2,g_o3SwitchIndex);//开2个
                        g_touch = 12;
                        g_AutoOsctime = kc*g_lenth*g_width*g_height/1000.0;   //震荡总时间 KC *gv
                    }
                    else  //时间未到
                    {
                       g_status = STATUS_AUTOV_RUN_F4; //等待2分钟
                    }
                    break;

                case STATUS_AUTOV_RUN_F4: //等待2分钟 ，再次计算
                    g_timerun++;
                   if(g_timerun>AUTOVCALTIME)
                   {
                      kp = FunO3CalParameter();
                      g_AutoVtime = g_AutoVtime - AUTOVCALTIME*kp; //T2 = T1 -120*KP
                      printf("kp = %f, kv=%f, the time is %f osc time =%f\r\n",kp,kv,g_AutoVtime,kc*g_lenth*g_width*g_height/1000.0);
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_F3; //继续回去计算时间
                   }
                    break;
                case STATUS_AUTOV_RUN_ON://震荡模式 开阶段
                     kp = FunO3CalParameter();
                     g_AutoOsctime = g_AutoOsctime-kp;
                     printf("OSC TIME = %5.1f\r\n",g_AutoOsctime);
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>AUTOVONTIME)  //固定2分钟
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_OFF; //进入震荡 关阶段
                      FunO3Control(0,g_o3SwitchIndex);
                   }
                   if(g_AutoOsctime<2)
                  // if(g_AutoVEntercount>g_AutoOsctime)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       FunO3Control(0,1);
                       g_status = STATUS_AUTOV_WAIT_D1;
                   }
                    break;
                case STATUS_AUTOV_RUN_OFF://震荡模式 关阶段
                     kp = FunO3CalParameter();
                     g_AutoOsctime = g_AutoOsctime-kp;
                     printf("OSC TIME = %5.1f\r\n",g_AutoOsctime);
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>koscoff*100000.0/(g_lenth*g_width*g_height/1000.0)) //震荡关时间与体积成反比  0.9345*100000/g_V 
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_ON; //进入震荡 开阶段
                      FunO3Control(2,1);
                      
                   }
                  //  if(g_AutoVEntercount>g_AutoOsctime)
                   if(g_AutoOsctime<2)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       g_status = STATUS_AUTOV_WAIT_D1;
                       FunO3Control(0,1);
                   }
                    break;
                 case STATUS_AUTOV_WAIT_D1://等待待机
                     g_timerun++;
                     if(g_timerun>FANOFFTIME)
                   {
                      g_timerun =0;
                      if((g_chemical ==1) &&(g_secondrun==0))//带芳香
                      {
                          g_secondrun =1;
                           g_status = STATUS_AUTOV_START; //重新开始一次
                      }
                      else
                      {
                        g_status = STATUS_AUTOV_WAIT; //进入待机状态
                        FunFanClose();
                      }
                     
                      
                   }
                   
                    break;
                  case STATUS_AUTOV_WAIT:
                     g_timerun++;
                     bsp_ScreenUPdateSYSSTATUS(g_status);  

                     if(g_timerun>5)
                     {
                         g_timerun =0;
                         g_status = STATUS_AUTOV_STOP_D1;
                     }
                    break;
                 
                     
                  case STATUS_AUTOV_STOP_D1://待机中
                     g_timerun++;
                   
                     if(g_timerun %5 ==0)
                     {
                       bsp_PlayMusic();  
                         
                     }
                     
                    break;
                
                 
//---------------------------自动体积模式 状态轮询------------------end----------------------------//    
                default:
                    break;
            
        }
        g_funcKey=0xff;  //g_func 恢复初始值
}
//-----------------------FunSwitch--------状态切换 end-------------------------//


//系统自检函数 ，检测系统运行是否ok 
void bsp_SysCheck(void)
{
//    uint8_t tempsec;
//    RTC_ReadClock();
//    printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, 
//                  g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
//    while(1)
//    {
//       	/* 读时钟，结果存放在全局变量 g_tRTC */
//        if(tempsec != g_tRTC.Sec)
//        {
//            RTC_ReadClock();
//            tempsec=g_tRTC.Sec;
//            switch(g_status)
//            {   
//                case  STATUS_CHECK_F1:
//                    DHT11_ReadData(&g_DHT11);
//                    if(g_DHT11.Hum>=850) //>=85%
//                    {
//                        
//                    }
//                    else if(g_DHT11.Hum>650)//>=65% <=85%
//                    {
//                        
//                    }
//                    else if(g_DHT11.Hum>550)//>=55% <= 65%
//                    {
//                        
//                    }
//                    break;
//                case  STATUS_CHECK_F2:
//                    break;
//                case  STATUS_CHECK_F3:
//                    break;
//                case  STATUS_CHECK_F4:
//                    break;
//                case  STATUS_CHECK_F5:
//                    break;
//                default:
//                    break;
//                
//            }
//        }
//    }
}

void FunTouch(void)
{
    if(g_touch==1)   //输入密码正确时
    {
        bsp_SendKey(3); //跳转到设置体积
        bsp_DelayMS(100);
        bsp_SendKey(3);
        bsp_DelayMS(1);
        g_touch = 0xff;
            
    }
    else if(g_touch==2)  //输入密码错误时
    {
        bsp_SendKey(4); //回到状态界面
        bsp_DelayMS(5);
        bsp_SendKey(4); //回到状态界面
        bsp_DelayMS(5);
        bsp_DisableTouch();//关闭触摸
        g_touch = 0xff;
    }
    else if(g_touch==3)  //设置--确定按钮
    {
        bsp_DisableTouch(); //关闭触摸
        bsp_DelayMS(50);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(50);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(10);
        g_touch = 0xff;
    }
    else if(g_touch==4)  //设置--开启网络
    {
        eeResult[7] = 0;  //复位失败次数
        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //写失败次数
        nbiot_reset();
        g_touch = 0xff;
    }
    else if(g_touch==5)  //设置--关闭网络
    {
        eeResult[7] = 60;  //复位失败次数
        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //写失败次数
        nbiot_reset();
        g_touch = 0xff;
    }
    else if(g_touch==6) //按键-启动密码界面
    {
        bsp_SendKey(0x02);  
        bsp_DelayMS(100); 
        bsp_SendKey(0x02);  
        bsp_DelayMS(100);
        bsp_EnableTouch();
        bsp_DelayMS(100);
        bsp_EnableTouch();
        bsp_DelayMS(10);
        g_touch = 0xff;
 
 
    }
    else if((g_touch==7)||(g_touch ==8)) //芳香类型 更新
    {
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
                    
        eeResult[1] = g_chemical;
        eeResult[0] |= 0x10;                       //标志 化学类型已经设置
        ee_WriteBytes(0,eeResult,0x00,0x01);       //写标志
        ee_WriteBytes(0,&eeResult[1],0x0f,0x01);   //写化学类型
        
        g_touch = 0xff;
     }
    
     else if((g_touch==9)||(g_touch ==10)) //植株类型 更新
    {
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        
        eeResult[1] = g_plant;
        eeResult[0] |= 0x08;                       //标志 植株类型已经设置
        ee_WriteBytes(0,eeResult,0x00,0x01);       //写标志
        ee_WriteBytes(0,&eeResult[1],0x0e,0x01);   //写植株类型
        
        g_touch = 0xff;
     }
    else if((g_touch == 11)||(g_touch == 12))  //更新显示屏  开始运行
    {
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        g_touch = 0xff;
    }
    else if(g_touch == 13)  //更新显示屏  开始运行
    {
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        g_touch = 0xff;
    }
    
}


