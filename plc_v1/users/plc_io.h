/****************************************************************************
PLC相关的特殊寄存器
专用辅助继电器                 描述
M8126                          全局标志
M8127                          通讯请求握手信号
M8128                          出错标志 
M8129                          通讯请求切换        
   
专用数据寄存器                描述
D8000 = 200;		   		        扫描时间
D8001 = 0X5EF6;	              型号版本 FX2N(C)
D8101 = 0X5EF6;	              型号版本 FX2N(C)
D8002 = 8;			   		        内存容量
D8102 = 8;			   		        内存容量
D8003 = 0x0010; 	   	        内存类型、寄存器类型
D8006                         CPU电池电压
D8010 = 10;			   	          扫描当前值
D8011 = 20;			   	          扫描最小时间(0.1MS)
D8012 = 140;			   	        扫描最长时间(0.1MS) 

D8120 = 0X4096                通讯格式
D8121                         从站号（最多16个）
D8127                         交换数据的首地址
D8128                         交换数据量
D8129                         网络通讯超时时间确认值
D8000                         看门狗         

通讯格式详解（D8120）
----------------------------------------------------------------------
位号	     |   含 义	   |          描述      
-----------+-------------+--------------------------------------------
b0	       |  数据长度	 |   0： 7位   1： 8位
-----------+-------------+--------------------------------------------
b2b1	     |  校验方式	 |   00：不用  01：奇校验  11：偶校验
-----------+-------------+--------------------------------------------
b3	       |   停止位	   |   0： 1位   1： 2位
-----------+-------------+--------------------------------------------
           |             |   0001：300      0111：4800
b7b6b5b4	 |   波特率	   |   0100：600      1000：9600
           |             |   0101：1200     1001：19200
           |             |   0110：2400
-----------+-------------+--------------------------------------------
b8		     |             |   0：不用   注：无协议通讯专用
-----------+-------------+--------------------------------------------
b9		     |             |   0：不用   同上
-----------+-------------+--------------------------------------------
b12b11b10	 |  通讯接口	 |   000：RS485（RS422）接口
           |             |   010：        RS232C接口
-----------+-------------+--------------------------------------------
b13	       |  求和检查	 |   0：不加求和码  1：自动加上求和码
-----------+-------------+-------------------------------------------
b14	       |   协议	     |   0：无协议通讯  1：专用通讯协议
-----------+-------------+--------------------------------------------
b15	       | 协议格式	   |   0：格式1  1：格式4				 
----------------------------------------------------------------------

举例：D8120 = 0X4096           通讯波特率是19200

*********************************************************************************/
#ifndef _PLC_IO_H 
#define _PLC_IO_H 

#include "stm32f10x.h"              //0X4221018C

#ifdef __cplusplus
extern "C"  {
#endif
#ifdef __cplusplus
}
#endif

#define BIT_ADDR(addr, bitnum)         *((volatile unsigned long  *)((0x42000000)+(addr<<5)+(bitnum<<2)))                   //
//IO口地址映射                                                                
#define GPIOA_ODR_Addr    0x01080C  
#define GPIOB_ODR_Addr    0x010C0C 
#define GPIOC_ODR_Addr    0x01100C 
#define GPIOD_ODR_Addr    0x01140C 
//IO模式定义
#define GPIOA_IDR_Addr    0x010808 
#define GPIOB_IDR_Addr    0x010C08 
#define GPIOC_IDR_Addr    0x011008 
#define GPIOD_IDR_Addr    0x011408 
//IO口操作,只对单一的IO口!

//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PLC_RUN    PCin(13)
#define PVD        PAin(0)

#define run_Lamp    PBout(8)=0
#define Stop_Lamp   PBout(8)=1 

#define Err_Lamp_ON    PBout(9)=0
#define Err_Lamp_OFF   PBout(9)=1 

// #define send_485    PDout(10)=1         //485  mode open send
// #define read_485    PDout(10)=0         //485  mode open read

//输入X定义
#define X00 PAin(1)	
#define X01 PAin(2)
#define X02 PAin(3)
#define X03 PAin(6)
#define X04 PAin(7)		 
#define X05 PCin(4)
#define X06 PCin(5)
#define X07 PBin(0)
#define X10 PBin(1)		
#define X11 PBin(12)
#define X12 PBin(13)
#define X13 PBin(14)
#define X14 PBin(15)
	
#define X15 PBin(7)
#define X16 PBin(8)
#define X17 PBin(9)

// 用户可根据实际需求进行扩展
//#define X20  管脚待定 		  
//#define X21  管脚待定 		  
//#define X22  管脚待定 		
//#define X23  管脚待定 		
//#define X24  管脚待定 		
//#define X25  管脚待定 		
//#define X26  管脚待定 		
//#define X27  管脚待定 		
//#define X30  管脚待定 		 
//#define X31  管脚待定 		 
//#define X32  管脚待定 		 
//#define X33  管脚待定 		 
//#define X34  管脚待定 		 	
//#define X35  管脚待定 		 
//#define X36  管脚待定 		
//#define X37  管脚待定 		



//输出Y定义
#define Y00 PCout(9)  
#define Y01 PAout(8)  
#define Y02 PAout(11)
#define Y03 PAout(12)
#define Y04 PAout(15)   
#define Y05 PCout(10) 
#define Y06 PCout(11) 
#define Y07 PCout(12)

#define Y10 PDout(2 )  
#define Y11 PBout(3 )
#define Y12 PBout(4 )
#define Y13 PBout(5 )
#define Y14 PBout(6 )
#define Y15 PBout(7 )

//#define Y16 PAout(1 )
//#define Y17 PAout(0 )

// 用户可根据实际需求进行扩展
//#define Y20  管脚待定    
//#define Y21  管脚待定 
//#define Y22  管脚待定 
//#define Y23  管脚待定
//#define Y24  管脚待定   
//#define Y25  管脚待定 
//#define Y26  管脚待定 
//#define Y27  管脚待定
//#define Y30  管脚待定      
//#define Y31  管脚待定 
//#define Y32  管脚待定 
//#define Y33  管脚待定 
//#define Y34  管脚待定      
//#define Y35  管脚待定 
//#define Y36  管脚待定 
//#define Y37  管脚待定 


#endif

