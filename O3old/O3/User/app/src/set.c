#include "bsp.h"
uint8_t g_status =0;   //系统运行的所有状态切换
uint8_t g_WithoutOnenet=0; //不使用onenet 标志  =1 表示 不使用ONENET
uint8_t g_plant=0; //植株类型  0  幼体   1 成熟体
uint8_t g_chemical =0 ; // 化学类型 0不带芳香  1带芳香
uint8_t g_checkfreok[4]={0,0,0,0};
uint8_t g_touch=0xff; //触摸处理标志
float kv = 0.75; //体积参数
float kc = 1.0; //震荡总时间参数  kc*体积 
float kp = 1.0; //计算的参数时间 kh*kt
uint32_t g_sysruntime =0;
void LoadSysPara(void)
{
    
}