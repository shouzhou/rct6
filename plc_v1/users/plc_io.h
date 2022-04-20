/****************************************************************************
PLC��ص�����Ĵ���
ר�ø����̵���                 ����
M8126                          ȫ�ֱ�־
M8127                          ͨѶ���������ź�
M8128                          �����־ 
M8129                          ͨѶ�����л�        
   
ר�����ݼĴ���                ����
D8000 = 200;		   		        ɨ��ʱ��
D8001 = 0X5EF6;	              �ͺŰ汾 FX2N(C)
D8101 = 0X5EF6;	              �ͺŰ汾 FX2N(C)
D8002 = 8;			   		        �ڴ�����
D8102 = 8;			   		        �ڴ�����
D8003 = 0x0010; 	   	        �ڴ����͡��Ĵ�������
D8006                         CPU��ص�ѹ
D8010 = 10;			   	          ɨ�赱ǰֵ
D8011 = 20;			   	          ɨ����Сʱ��(0.1MS)
D8012 = 140;			   	        ɨ���ʱ��(0.1MS) 

D8120 = 0X4096                ͨѶ��ʽ
D8121                         ��վ�ţ����16����
D8127                         �������ݵ��׵�ַ
D8128                         ����������
D8129                         ����ͨѶ��ʱʱ��ȷ��ֵ
D8000                         ���Ź�         

ͨѶ��ʽ��⣨D8120��
----------------------------------------------------------------------
λ��	     |   �� ��	   |          ����      
-----------+-------------+--------------------------------------------
b0	       |  ���ݳ���	 |   0�� 7λ   1�� 8λ
-----------+-------------+--------------------------------------------
b2b1	     |  У�鷽ʽ	 |   00������  01����У��  11��żУ��
-----------+-------------+--------------------------------------------
b3	       |   ֹͣλ	   |   0�� 1λ   1�� 2λ
-----------+-------------+--------------------------------------------
           |             |   0001��300      0111��4800
b7b6b5b4	 |   ������	   |   0100��600      1000��9600
           |             |   0101��1200     1001��19200
           |             |   0110��2400
-----------+-------------+--------------------------------------------
b8		     |             |   0������   ע����Э��ͨѶר��
-----------+-------------+--------------------------------------------
b9		     |             |   0������   ͬ��
-----------+-------------+--------------------------------------------
b12b11b10	 |  ͨѶ�ӿ�	 |   000��RS485��RS422���ӿ�
           |             |   010��        RS232C�ӿ�
-----------+-------------+--------------------------------------------
b13	       |  ��ͼ��	 |   0�����������  1���Զ����������
-----------+-------------+-------------------------------------------
b14	       |   Э��	     |   0����Э��ͨѶ  1��ר��ͨѶЭ��
-----------+-------------+--------------------------------------------
b15	       | Э���ʽ	   |   0����ʽ1  1����ʽ4				 
----------------------------------------------------------------------

������D8120 = 0X4096           ͨѶ��������19200

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
//IO�ڵ�ַӳ��                                                                
#define GPIOA_ODR_Addr    0x01080C  
#define GPIOB_ODR_Addr    0x010C0C 
#define GPIOC_ODR_Addr    0x01100C 
#define GPIOD_ODR_Addr    0x01140C 
//IOģʽ����
#define GPIOA_IDR_Addr    0x010808 
#define GPIOB_IDR_Addr    0x010C08 
#define GPIOC_IDR_Addr    0x011008 
#define GPIOD_IDR_Addr    0x011408 
//IO�ڲ���,ֻ�Ե�һ��IO��!

//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //��� 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //���� 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //��� 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //���� 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //��� 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //���� 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //��� 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //���� 

#define PLC_RUN    PCin(13)
#define PVD        PAin(0)

#define run_Lamp    PBout(8)=0
#define Stop_Lamp   PBout(8)=1 

#define Err_Lamp_ON    PBout(9)=0
#define Err_Lamp_OFF   PBout(9)=1 

// #define send_485    PDout(10)=1         //485  mode open send
// #define read_485    PDout(10)=0         //485  mode open read

//����X����
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

// �û��ɸ���ʵ�����������չ
//#define X20  �ܽŴ��� 		  
//#define X21  �ܽŴ��� 		  
//#define X22  �ܽŴ��� 		
//#define X23  �ܽŴ��� 		
//#define X24  �ܽŴ��� 		
//#define X25  �ܽŴ��� 		
//#define X26  �ܽŴ��� 		
//#define X27  �ܽŴ��� 		
//#define X30  �ܽŴ��� 		 
//#define X31  �ܽŴ��� 		 
//#define X32  �ܽŴ��� 		 
//#define X33  �ܽŴ��� 		 
//#define X34  �ܽŴ��� 		 	
//#define X35  �ܽŴ��� 		 
//#define X36  �ܽŴ��� 		
//#define X37  �ܽŴ��� 		



//���Y����
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

// �û��ɸ���ʵ�����������չ
//#define Y20  �ܽŴ���    
//#define Y21  �ܽŴ��� 
//#define Y22  �ܽŴ��� 
//#define Y23  �ܽŴ���
//#define Y24  �ܽŴ���   
//#define Y25  �ܽŴ��� 
//#define Y26  �ܽŴ��� 
//#define Y27  �ܽŴ���
//#define Y30  �ܽŴ���      
//#define Y31  �ܽŴ��� 
//#define Y32  �ܽŴ��� 
//#define Y33  �ܽŴ��� 
//#define Y34  �ܽŴ���      
//#define Y35  �ܽŴ��� 
//#define Y36  �ܽŴ��� 
//#define Y37  �ܽŴ��� 


#endif

