

#ifndef __APP_FUNC_H
#define __APP_FUNC_H

#include "stdint.h"

//以下是 串口屏中 对应的按钮状态变量 按下时 会返回对应的数据
//0	启动手动模式	0x000A	1	启动手动模式			0
//0	启动循环模式	0x000A	1	启动循环模式			1
//4	启动自动浓度	0x000A	1	启动自动浓度			3
//4	停止自动浓度	0x000A	1	停止自动浓度			4
//4	启动自动体积	0x000A	1	启动自动体积			5
//4	停止自动体积	0x000A	1	停止自动体积			6
//3	停止手动模式	0x000A	1	停止手动模式			2
//5	停止循环模式	0x000A	1	停止循环模式			7
#define  STARTMANUAL   0
#define  STARTLOOP     1
#define  STOPMANUAL    2
#define  STARTAUTODEN  3
#define  STOPAUTODEN   4
#define  STARTAUTOV    5
#define  STOPAUTOV     6
#define  STOPLOOP      7


// 以下时自定义的各个状态变量 
#define  STATUS_ALL_STOP        0 
#define  STATUS_MANUAL_START    1
#define  STATUS_MANUAL_STOP     2
#define  STATUS_MANUAL_RUN_D1    10
#define  STATUS_MANUAL_STOP_D1   11 //延迟关闭阶段

#define  STATUS_LOOP_START      3 
#define  STATUS_LOOP_STOP       4 
#define  STATUS_LOOP_RUN_F1     30  //运行开阶段
#define  STATUS_LOOP_RUN_F2     31  //运行关阶段
#define  STATUS_LOOP_STOP_D1     32 //延迟关闭阶段


#define  STATUS_AUTODEN_START       5 
#define  STATUS_AUTODEN_STOP        6 
#define  STATUS_AUTODEN_RUN_F1      50  //自动浓度模式第一浓度阶段
#define  STATUS_AUTODEN_RUN_F2      51  //自动浓度模式第二浓度阶段
#define  STATUS_AUTODEN_RUN_F3      52  //自动浓度模式第三浓度阶段
#define  STATUS_AUTODEN_RUN_F4      53  //自动浓度模式第四浓度阶段
#define  STATUS_AUTODEN_RUN_F4_ON   54  //自动浓度模式第四浓度阶段_开
#define  STATUS_AUTODEN_RUN_F4_OFF  55  //自动浓度模式第四浓度阶段_关
#define  STATUS_AUTODEN_WAIT_D1     56  //待机延迟阶段
#define  STATUS_AUTODEN_STOP_D1     57  //延迟关闭阶段
#define  STATUS_AUTODEN_WAIT        58  //待机阶段






#define  STATUS_AUTOV_START     7 
#define  STATUS_AUTOV_STOP      8
#define  STATUS_AUTOV_RUN_F1    71
#define  STATUS_AUTOV_RUN_F2    72
#define  STATUS_AUTOV_RUN_F3    73
#define  STATUS_AUTOV_RUN_F4    74
#define  STATUS_AUTOV_WAIT_D1    75
#define  STATUS_AUTOV_WAIT    76
#define  STATUS_AUTOV_STOP_D1    77

#define  STATUS_AUTOV_RUN_ON    79
#define  STATUS_AUTOV_RUN_OFF   80






//void FuncManual(uint16_t mode);
//void FuncAutoV(uint16_t mode);
//void FuncAutoNH3(uint16_t mode);
//void FuncLoop(uint16_t mode);
void FunSwitch(void);
void FunFanOpen(void);
void FunFanClose(void);
void FunO3Control(uint8_t onnum,uint8_t index);
float FunO3CalParameter(void);



#endif

 
