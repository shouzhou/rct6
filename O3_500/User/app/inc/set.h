
#ifndef __SET_APP_H
#define __SET_APP_H
#include "stdint.h"

#define NETUPDATETIME        60   //有网络时更新间隔
#define NONETUPDATETIME      10    //无网络时更新间隔
////////自检参数////////////////////////////

#define CHECKSTARTTIME        10     //起始的时间  单位S
#define CHECKL1TIME           10      //自检阶段最高湿度延时时间
#define CHECKL2TIME           10      //自检阶段第二湿度延时时间
#define CHECKL3TIME           10      //自检阶段第三最高湿度延时时间
#define CHECKL4TIME           3       //自检阶段第四湿度延时时间
//#define WAITDELYTIME          600    //完成后风机继续运行时间

#define CHECKL1HUM           850    //自检阶段最高湿度   10倍关系
#define CHECKL2HUM           650    //自检阶段第二湿度
#define CHECKL3HUM           550    //自检阶段第三湿度
#define CHECKL4HUM           500    //自检阶段第四湿度

#define CHECKFRELOW          8      //自检阶段频率低值   单位KHZ         
#define CHECKFREHIGH         20      //自检阶段频率低值


//////运行参数
#define FANOFFTIME     120          //修改这个宏，改变风机延迟关闭时间  单位S
#define AUTOVONTIME    120          //修改这个宏，改变自动体积模式下震荡开时间
//#define AUTOVOFFTIME   120        //修改这个宏，改变自动体积模式下震荡关时间
#define AUTOVCALTIME   60           //修改这个宏，改变自动体积模式下每次计算参数的间隔时间 需要>5 否则会有问题 

//体积分段--10倍
#define KVLEVEL1  4000
#define KVLEVEL2  3750
#define KVLEVEL3  3500
#define KVLEVEL4  3250
#define KVLEVEL5  3000
#define KVLEVEL6  2750
#define KVLEVEL7  2500
#define KVLEVEL8  2250
#define KVLEVEL9  2000
#define KVLEVEL10  1750
#define KVLEVEL11  1500
#define KVLEVEL12  1250
#define KVLEVEL13  1000
#define KVLEVEL14  750
#define KVLEVEL15  500
#define KVLEVEL16  250

// 体积分段对应的参数
#define KVL1       0.3
#define KVL2       0.55
#define KVL3       0.7
#define KVL4       0.9
#define KVL5       1
#define KVL6       1.1
#define KVL7       1.2
#define KVL8       1.3
#define KVL9       1.3
#define KVL10       1.3
#define KVL11       1.3
#define KVL12       1.3
#define KVL13       1.3
#define KVL14       1.3
#define KVL15       1.3
#define KVL16       1.3

#define KVOTHER    1.4




// 体积分段对应的参数
#define KVOSC1       0.3
#define KVOSC2       0.55
#define KVOSC3       0.7
#define KVOSC4       0.9
#define KVOSC5       1
#define KVOSC6       1.1
#define KVOSC7       1.2
#define KVOSC8       1.3
#define KVOSC9       1.3
#define KVOSC10       1.3
#define KVOSC11       1.3
#define KVOSC12       1.3
#define KVOSC13       1.3
#define KVOSC14       1.3
#define KVOSC15       1.3
#define KVOSC16       1.3

#define KVOSCOTHER    1.4

//湿度分段---10倍
#define HLEVEL1    950 
#define HLEVEL2    850 
#define HLEVEL3    800 
#define HLEVEL4    750 
#define HLEVEL5    650 
#define HLEVEL6    600 
#define HLEVEL7    550 
#define HLEVEL8    500 
//不同湿度对应的参数
#define KHL1       0.3
#define KHL2       0.35
#define KHL3       0.4
#define KHL4       0.5
#define KHL5       0.6
#define KHL6       0.8
#define KHL7       0.9
#define KHL8       1
#define KHOTHER    1

//温度分段--10倍
#define TLEVEL1    550 
#define TLEVEL2    500 
#define TLEVEL3    450 
#define TLEVEL4    400 
#define TLEVEL5    350 
#define TLEVEL6    300 
#define TLEVEL7    250 
#define TLEVEL8    200 
#define TLEVEL9    100 
#define TLEVEL10   50

//不同温度对应的参数
#define KTL1    0.8
#define KTL2    0.8
#define KTL3    0.8
#define KTL4    0.8
#define KTL5    0.8
#define KTL6    0.8
#define KTL7    0.8
#define KTL8    0.98
#define KTL9    1
#define KTL10   1
#define KTOTHER 1

//按键功能
#define  STARTAUTOV                 1
#define  STOPAUTOV                  2

// 以下时自定义的各个状态变量 

#define  STATUS_ALL_STOP            0 
#define  STATUS_SYSINIBEGIN         1 
#define  STATUS_SYSINIEND           2 
#define  STATUS_NETINIBEGIN         3 
#define  STATUS_NETINIEND           4 
#define  STATUS_NETFAIL             5
#define  SYSINIFINISH               6




#define STATUS_AUTOV_START          10 
#define STATUS_CHECK_F1_1           11
#define STATUS_CHECK_F1_2           12
#define STATUS_CHECK_F2             13
#define STATUS_CHECK_F3_L1          14
#define STATUS_CHECK_F3_L2          15
#define STATUS_CHECK_F3_L3          16
#define STATUS_CHECK_F3_L4          17
#define STATUS_CHECK_F3_L5          18
#define STATUS_CHECK_F4             19
#define STATUS_CHECK_OK             20
#define STATUS_CHECK_FAIL           21
#define  STATUS_AUTOV_STOP          22
#define  STATUS_AUTOV_RUN_F1        23
#define  STATUS_AUTOV_RUN_F2        24
#define  STATUS_AUTOV_RUN_F3        25
#define  STATUS_AUTOV_RUN_F3_2MIN   26
#define  STATUS_AUTOV_RUN_F4        27


#define  STATUS_AUTOV_RUN_ON        28
#define  STATUS_AUTOV_RUN_OFF       29
#define  STATUS_AUTOV_WAIT_D1       30
#define  STATUS_AUTOV_WAIT          31
#define  STATUS_AUTOV_STOP_D1       32


extern uint8_t g_status; 
extern uint8_t g_WithoutOnenet;
extern uint8_t g_plant; //植株类型  0  幼体   1 成熟体
extern uint8_t g_chemical  ; // 化学类型 0不带芳香  1带芳香
extern uint8_t g_checkfreok[4];
extern uint8_t g_touch; //触摸处理标志
extern float kp,kv,kc,kvosc;
extern float g_AutoVtime;
extern uint32_t g_sysruntime ;
extern float koscoff ;
extern uint8_t g_secondrun;
void LoadSysPara(void);

#endif

 
