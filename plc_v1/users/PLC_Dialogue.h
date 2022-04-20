/************************************************************
库名称:    PLC_Dialogue
功能描述： PLC宏处理
作　者:    传人记
日　期:    2015年7月22日
版  本:    1.1
************************************************************/
#ifndef __PLC_Dialogue_H__
#define __PLC_Dialogue_H__
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C"  {
#endif
extern const unsigned char PLC_BIT_OR[];
extern const unsigned char PLC_BIT_AND[];
extern u8  PLC_8BIT[4];               //PLC_RAM运行数据
extern u8  Step_Address[2000];        //为PLS OR PLF use	
#ifdef __cplusplus
}
#endif

#define PLC_RAM_ADDR    0x8006000	  //起始地址必需为0X800000开始加 0X1000 整倍数,因为后面写PLC程序时需要用到块擦除
#define RAM_ADDR        0x20001000    //PLC_RAM起始地址
#define RAM_T_ADDR      0x20002000    //PLC T 起始地址
#define RAM_C_ADDR      0x20001A00    //PLC C 起始地址
#define RAM_D_ADDR      0x20003000    //PLC D 起始地址
#define RAM_D1000_ADDR  0x200037D0    //PLC D1000 起始地址
#define RAM_D8000_ADDR  0x20001E00    //PLC D8000 起始地址
#define PLC_START_Address   ((u16*)(0x800605c))//PLC程序开始地址
#define ROTATE_LEFT(x, s, n)        ((x) << (n)) | ((x) >> ((s) - (n)))      //循环左移  x为数据 s为数据位数 n为移动位数
#define ROTATE_RIGHT(x, s, n)       ((x) >> (n)) | ((x) << ((s) - (n)))      //循环右移  x为数据 s为数据位数 n为移动位数

#define swap_u16(x)       ((x) >> (8)) | ((x) << (8))            //上下交换传送
#define swap_u32(x)       ((x) >> (16))|((x) << (16))            //上下交换传送

#define	PLC_D_C_T_addr(x)  ((x) % (0x4000))         //V,Z的时候用

#define	PLC_v_z_addr(x)    ((x) / (0x4000))         //V,Z的时候用

#define PLC_RAM8(x)    (*(u8*)(u32)(x))		          //字节方式	R/W RAM	R ROM

#define PLC_RAM16(x)   (*(s16*)(u32)(x))				    //半字方式	R/W RAM	R ROM

#define PLC_RAM32(x)   (*(s32*)(u32)(x))				    //字方式	R/W RAM	R ROM

#define PLC_RAM64(x)   (*(int64_t*)(u32)(x))				//字方式	R/W RAM	R ROM

#define PLC_RAMfolta(x)   (*(float*)(u32)(x))				//字方式	R/W RAM	R ROM

#define trade1         FLOAT.DATA                   //浮点输出的时候用

#define PLC_8BIT(x)     PLC_RAM8(RAM_ADDR+x)

#define PLC_BIT_TEST(x)	    (PLC_RAM8(RAM_ADDR+((x)/8)) & PLC_BIT_OR[(x)%8])

#define PLC_BIT_ON(x)		  (PLC_RAM8(RAM_ADDR+((x)/8))	|=PLC_BIT_OR[(x)%8])
#define PLC_BIT_OFF(x)		(PLC_RAM8(RAM_ADDR+((x)/8))	&=PLC_BIT_AND[(x)%8])

#define PLC_PL_BIT_ON(x)	 (Step_Address[(x)/8] |=PLC_BIT_OR[(x)%8]) //写状态值为1	   上升延与下降沿使用
#define PLC_PL_BIT_OFF(x)	 (Step_Address[(x)/8] &=PLC_BIT_AND[(x)%8])//写步状态为0上升延与下降沿使用共2K字节1600bit

#define on  1
#define off 0

typedef union
{
	struct
	{
		  u8 bit0		:1;
  		u8 bit1		:1;
  		u8 bit2		:1;
  		u8 bit3		:1;
  		u8 bit4		:1;
  		u8 bit5		:1;
  		u8 bit6		:1;
  		u8 bit7		:1;
		  u8 bit10		:1;
		  u8 bit11		:1;
		  u8 bit12		:1;
		  u8 bit13		:1;
		  u8 bit14		:1;
		  u8 bit15		:1;	
			u8 bit16		:1;
		  u8 bit17		:1;
	}bits;	            //可以按位域寻址
  u16 bytes;       	  //可以按字节寻址
}bit_byte;  		      //定义一个既能按位域寻址也可按字节寻址的新变量类型
/*
 *16位整型共同体
 */
typedef union
{
  u8  PLC_8BIT[24200];
	u16 PLC_16BIT[12100];
} union_16BIT;

/*
 *浮点共同体
 */
typedef union                                
{
  float DATA;
	u16   DATA1[2];
	u32   bata;
} float_union;
/*
 *32位整型共同体
 */
typedef union
{
  s32 data;
	s16 data1[2];
} s32_union;
/*
 *64位整型共同体
 */
typedef union
{
  int64_t data;
	u16 data1[4];
} u64_union;

#endif

#ifndef __at
#define __at(_addr) __attribute__ ((at(_addr)))
#endif

