/************************************************************
������:    PLC_Dialogue
���������� PLC�괦��
������:    ���˼�
�ա���:    2015��7��22��
��  ��:    1.1
************************************************************/
#ifndef __PLC_Dialogue_H__
#define __PLC_Dialogue_H__
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C"  {
#endif
extern const unsigned char PLC_BIT_OR[];
extern const unsigned char PLC_BIT_AND[];
extern u8  PLC_8BIT[4];               //PLC_RAM��������
extern u8  Step_Address[2000];        //ΪPLS OR PLF use	
#ifdef __cplusplus
}
#endif

#define PLC_RAM_ADDR    0x8006000	  //��ʼ��ַ����Ϊ0X800000��ʼ�� 0X1000 ������,��Ϊ����дPLC����ʱ��Ҫ�õ������
#define RAM_ADDR        0x20001000    //PLC_RAM��ʼ��ַ
#define RAM_T_ADDR      0x20002000    //PLC T ��ʼ��ַ
#define RAM_C_ADDR      0x20001A00    //PLC C ��ʼ��ַ
#define RAM_D_ADDR      0x20003000    //PLC D ��ʼ��ַ
#define RAM_D1000_ADDR  0x200037D0    //PLC D1000 ��ʼ��ַ
#define RAM_D8000_ADDR  0x20001E00    //PLC D8000 ��ʼ��ַ
#define PLC_START_Address   ((u16*)(0x800605c))//PLC����ʼ��ַ
#define ROTATE_LEFT(x, s, n)        ((x) << (n)) | ((x) >> ((s) - (n)))      //ѭ������  xΪ���� sΪ����λ�� nΪ�ƶ�λ��
#define ROTATE_RIGHT(x, s, n)       ((x) >> (n)) | ((x) << ((s) - (n)))      //ѭ������  xΪ���� sΪ����λ�� nΪ�ƶ�λ��

#define swap_u16(x)       ((x) >> (8)) | ((x) << (8))            //���½�������
#define swap_u32(x)       ((x) >> (16))|((x) << (16))            //���½�������

#define	PLC_D_C_T_addr(x)  ((x) % (0x4000))         //V,Z��ʱ����

#define	PLC_v_z_addr(x)    ((x) / (0x4000))         //V,Z��ʱ����

#define PLC_RAM8(x)    (*(u8*)(u32)(x))		          //�ֽڷ�ʽ	R/W RAM	R ROM

#define PLC_RAM16(x)   (*(s16*)(u32)(x))				    //���ַ�ʽ	R/W RAM	R ROM

#define PLC_RAM32(x)   (*(s32*)(u32)(x))				    //�ַ�ʽ	R/W RAM	R ROM

#define PLC_RAM64(x)   (*(int64_t*)(u32)(x))				//�ַ�ʽ	R/W RAM	R ROM

#define PLC_RAMfolta(x)   (*(float*)(u32)(x))				//�ַ�ʽ	R/W RAM	R ROM

#define trade1         FLOAT.DATA                   //���������ʱ����

#define PLC_8BIT(x)     PLC_RAM8(RAM_ADDR+x)

#define PLC_BIT_TEST(x)	    (PLC_RAM8(RAM_ADDR+((x)/8)) & PLC_BIT_OR[(x)%8])

#define PLC_BIT_ON(x)		  (PLC_RAM8(RAM_ADDR+((x)/8))	|=PLC_BIT_OR[(x)%8])
#define PLC_BIT_OFF(x)		(PLC_RAM8(RAM_ADDR+((x)/8))	&=PLC_BIT_AND[(x)%8])

#define PLC_PL_BIT_ON(x)	 (Step_Address[(x)/8] |=PLC_BIT_OR[(x)%8]) //д״ֵ̬Ϊ1	   ���������½���ʹ��
#define PLC_PL_BIT_OFF(x)	 (Step_Address[(x)/8] &=PLC_BIT_AND[(x)%8])//д��״̬Ϊ0���������½���ʹ�ù�2K�ֽ�1600bit

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
	}bits;	            //���԰�λ��Ѱַ
  u16 bytes;       	  //���԰��ֽ�Ѱַ
}bit_byte;  		      //����һ�����ܰ�λ��ѰַҲ�ɰ��ֽ�Ѱַ���±�������
/*
 *16λ���͹�ͬ��
 */
typedef union
{
  u8  PLC_8BIT[24200];
	u16 PLC_16BIT[12100];
} union_16BIT;

/*
 *���㹲ͬ��
 */
typedef union                                
{
  float DATA;
	u16   DATA1[2];
	u32   bata;
} float_union;
/*
 *32λ���͹�ͬ��
 */
typedef union
{
  s32 data;
	s16 data1[2];
} s32_union;
/*
 *64λ���͹�ͬ��
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

