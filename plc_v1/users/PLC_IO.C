/********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "PLC_IO.h"
#include "stm32f10x.h"
#include <stdio.h>
#include "absacc.h" 
#include "PLC_Dialogue.h"
extern u8 power_down;                  //断电检测程序
extern bit_byte PLC_16BIT[12100];
u8  X_DIY=10;	                        //指令修改滤波时间X000-X007
#define D8020 PLC_16BIT[0x0714].bytes //滤波时间X010以后的
//======================================================================================================
// 函数名称:  void PLC_X_config(void)   
// 功能描述： PLC_运行状态IO初始化
// 输　入:   void  
// 输　出:   void   
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月13日
// 备  注:  
//------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//=======================================================================================================
void PLC_EXTI15_10_config(void)
{ 
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource12|GPIO_PinSource11);
	EXTI_InitStructure.EXTI_Line=EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	EXTI_InitStructure.EXTI_Line=EXTI_Line11;
	EXTI_Init(&EXTI_InitStructure);	
}

//======================================================================================================
// 函数名称:  void PLC_X_config(void)   
// 功能描述： PLC_运行状态IO初始化
// 输　入:   void  
// 输　出:   void   
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月13日
// 备  注:  
//------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//=======================================================================================================
void PLC_EXTI9_5_config(void)
{ 
	EXTI_InitTypeDef EXTI_InitStructure;
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource9);
	EXTI_InitStructure.EXTI_Line=EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource8);
	EXTI_InitStructure.EXTI_Line=EXTI_Line8;
	EXTI_Init(&EXTI_InitStructure);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line=EXTI_Line7;
	EXTI_Init(&EXTI_InitStructure);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource6);
	EXTI_InitStructure.EXTI_Line=EXTI_Line6;
	EXTI_Init(&EXTI_InitStructure);		
}



//======================================================================================================
// 函数名称:  void PLC_X_config(void)   
// 功能描述： PLC_运行状态IO初始化
// 输　入:   void  
// 输　出:   void   
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月13日
// 备  注:  
//------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//=======================================================================================================
void PLC_Mode_config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	
  //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);陈凡注释
	  //define EEPROM
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;	   //EEPROM
// 	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_OD;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
// 	GPIO_Init(GPIOB,&GPIO_InitStructure);
	

    //define OTHER IO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	       //SHUTDOWN
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	       //RUN/STOP_SWITCH
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;  //RUN_LED  ERR_LED
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7 |GPIO_Pin_8; //文本显示
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;                 //PLC_485通讯
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

//======================================================================================================
// 函数名称:  void PLC_X_config(void)   
// 功能描述： PLC_X_输入初始化
// 输　入:   void  
// 输　出:   void   
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月13日
// 备  注:  
//------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//=======================================================================================================
void PLC_X_config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
  //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);陈凡注释	
//define X00-X14
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7;	
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);		

}

//======================================================================================================
// 函数名称:  PLC_Y_config(void)   
// 功能描述： PLC_Y_输出初始化
// 输　入:   void  
// 输　出:   void   
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月13日
// 备  注:  
//------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//=======================================================================================================
void PLC_Y_config(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
  //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); 陈凡注释	
//define Y00-Y15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void X_filter(void)			           //每　1ms  需调用一次，用于X滤波，暂定为20MS
{	 
	static signed char x_buffer[33];   //暂定刷新16个X 
	(X00) ? (x_buffer[0]=0,PLC_16BIT[64].bits.bit0=0) : (x_buffer[0]<X_DIY) ? (x_buffer[0]++) : (PLC_16BIT[64].bits.bit0=1);
	(X01) ? (x_buffer[1]=0,PLC_16BIT[64].bits.bit1=0) : (x_buffer[1]<X_DIY) ? (x_buffer[1]++) : (PLC_16BIT[64].bits.bit1=1);	
	(X02) ? (x_buffer[2]=0,PLC_16BIT[64].bits.bit2=0) : (x_buffer[2]<X_DIY) ? (x_buffer[2]++) : (PLC_16BIT[64].bits.bit2=1);	
	(X03) ? (x_buffer[3]=0,PLC_16BIT[64].bits.bit3=0) : (x_buffer[3]<X_DIY) ? (x_buffer[3]++) : (PLC_16BIT[64].bits.bit3=1);	
	(X04) ? (x_buffer[4]=0,PLC_16BIT[64].bits.bit4=0) : (x_buffer[4]<X_DIY) ? (x_buffer[4]++) : (PLC_16BIT[64].bits.bit4=1);	
	(X05) ? (x_buffer[5]=0,PLC_16BIT[64].bits.bit5=0) : (x_buffer[5]<X_DIY) ? (x_buffer[5]++) : (PLC_16BIT[64].bits.bit5=1);	
	(X06) ? (x_buffer[6]=0,PLC_16BIT[64].bits.bit6=0) : (x_buffer[6]<X_DIY) ? (x_buffer[6]++) : (PLC_16BIT[64].bits.bit6=1);	
	(X07) ? (x_buffer[7]=0,PLC_16BIT[64].bits.bit7=0) : (x_buffer[7]<X_DIY) ? (x_buffer[7]++) : (PLC_16BIT[64].bits.bit7=1);
	(X10) ? (x_buffer[8]=0,PLC_16BIT[64].bits.bit10=0) : (x_buffer[8]<D8020) ? (x_buffer[8]++) : (PLC_16BIT[64].bits.bit10=1);
	(X11) ? (x_buffer[9]=0,PLC_16BIT[64].bits.bit11=0) : (x_buffer[9]<D8020) ? (x_buffer[9]++) : (PLC_16BIT[64].bits.bit11=1);	
	(X12) ? (x_buffer[10]=0,PLC_16BIT[64].bits.bit12=0) : (x_buffer[10]<D8020) ? (x_buffer[10]++) : (PLC_16BIT[64].bits.bit12=1);	
	(X13) ? (x_buffer[11]=0,PLC_16BIT[64].bits.bit13=0) : (x_buffer[11]<D8020) ? (x_buffer[11]++) : (PLC_16BIT[64].bits.bit13=1);	
	(X14) ? (x_buffer[12]=0,PLC_16BIT[64].bits.bit14=0) : (x_buffer[12]<D8020) ? (x_buffer[12]++) : (PLC_16BIT[64].bits.bit14=1);	
 	(X15) ? (x_buffer[13]=0,PLC_16BIT[64].bits.bit15=0) : (x_buffer[13]<D8020) ? (x_buffer[13]++) : (PLC_16BIT[64].bits.bit15=1);	
 	(X16) ? (x_buffer[14]=0,PLC_16BIT[64].bits.bit16=0) : (x_buffer[14]<D8020) ? (x_buffer[14]++) : (PLC_16BIT[64].bits.bit16=1);	
 	(X17) ? (x_buffer[15]=0,PLC_16BIT[64].bits.bit17=0) : (x_buffer[15]<D8020) ? (x_buffer[15]++) : (PLC_16BIT[64].bits.bit17=1);
// 	
// 	(X20) ? (x_buffer[16]=0,PLC_16BIT[65].bits.bit0=0) : (x_buffer[16]<D8020) ? (x_buffer[16]++) : (PLC_16BIT[65].bits.bit0=1);
// 	(X21) ? (x_buffer[17]=0,PLC_16BIT[65].bits.bit1=0) : (x_buffer[17]<D8020) ? (x_buffer[17]++) : (PLC_16BIT[65].bits.bit1=1);	
// 	(X22) ? (x_buffer[18]=0,PLC_16BIT[65].bits.bit2=0) : (x_buffer[18]<D8020) ? (x_buffer[18]++) : (PLC_16BIT[65].bits.bit2=1);	
// 	(X23) ? (x_buffer[19]=0,PLC_16BIT[65].bits.bit3=0) : (x_buffer[19]<D8020) ? (x_buffer[19]++) : (PLC_16BIT[65].bits.bit3=1);	
//  (X24) ? (x_buffer[20]=0,PLC_16BIT[65].bits.bit4=0) : (x_buffer[20]<D8020) ? (x_buffer[20]++) : (PLC_16BIT[65].bits.bit4=1);	
//  (X25) ? (x_buffer[21]=0,PLC_16BIT[65].bits.bit5=0) : (x_buffer[21]<D8020) ? (x_buffer[21]++) : (PLC_16BIT[65].bits.bit5=1);	
//  (X26) ? (x_buffer[22]=0,PLC_16BIT[65].bits.bit6=0) : (x_buffer[22]<D8020) ? (x_buffer[22]++) : (PLC_16BIT[65].bits.bit6=1);	
//  (X27) ? (x_buffer[23]=0,PLC_16BIT[65].bits.bit7=0) : (x_buffer[23]<D8020) ? (x_buffer[23]++) : (PLC_16BIT[65].bits.bit7=1);
//  (X30) ? (x_buffer[24]=0,PLC_16BIT[65].bits.bit10=0) : (x_buffer[24]<D8020) ? (x_buffer[24]++) : (PLC_16BIT[65].bits.bit10=1);
//  (X31) ? (x_buffer[25]=0,PLC_16BIT[65].bits.bit11=0) : (x_buffer[25]<D8020) ? (x_buffer[25]++) : (PLC_16BIT[65].bits.bit11=1);	
//  (X32) ? (x_buffer[26]=0,PLC_16BIT[65].bits.bit12=0) : (x_buffer[26]<D8020) ? (x_buffer[26]++) : (PLC_16BIT[65].bits.bit12=1);	
//  (X33) ? (x_buffer[27]=0,PLC_16BIT[65].bits.bit13=0) : (x_buffer[27]<D8020) ? (x_buffer[27]++) : (PLC_16BIT[65].bits.bit13=1);	
//  (X34) ? (x_buffer[28]=0,PLC_16BIT[65].bits.bit14=0) : (x_buffer[28]<D8020) ? (x_buffer[28]++) : (PLC_16BIT[65].bits.bit14=1);	
//  (X35) ? (x_buffer[29]=0,PLC_16BIT[65].bits.bit15=0) : (x_buffer[29]<D8020) ? (x_buffer[29]++) : (PLC_16BIT[65].bits.bit15=1);	
//  (X36) ? (x_buffer[30]=0,PLC_16BIT[65].bits.bit16=0) : (x_buffer[30]<D8020) ? (x_buffer[30]++) : (PLC_16BIT[65].bits.bit16=1);	
//  (X37) ? (x_buffer[31]=0,PLC_16BIT[65].bits.bit17=0) : (x_buffer[31]<D8020) ? (x_buffer[31]++) : (PLC_16BIT[65].bits.bit17=1);
	//上电断电检测	
// 	if(!PVD) {if(power_down<=7)power_down++;}else{if(power_down>0)power_down--;}
}


//刷新输出，一个地址为32个点
void PLC_IO_Refresh(void)         
{

// 	Y00=~PLC_16BIT[80].bits.bit0;
// 	Y01=~PLC_16BIT[80].bits.bit1;
// 	Y02=~PLC_16BIT[80].bits.bit2;
// 	Y03=~PLC_16BIT[80].bits.bit3;
// 	Y04=~PLC_16BIT[80].bits.bit4;
// 	Y05=~PLC_16BIT[80].bits.bit5;
// 	Y06=~PLC_16BIT[80].bits.bit6;
// 	Y07=~PLC_16BIT[80].bits.bit7;
// 	Y10=~PLC_16BIT[80].bits.bit10;
// 	Y11=~PLC_16BIT[80].bits.bit11;
// 	Y12=~PLC_16BIT[80].bits.bit12;
// 	Y13=~PLC_16BIT[80].bits.bit13;
// 	Y14=~PLC_16BIT[80].bits.bit14;
// 	Y15=~PLC_16BIT[80].bits.bit15;
// 	Y16=~PLC_16BIT[80].bits.bit16;
// 	Y17=~PLC_16BIT[80].bits.bit17;
// 	
// 	Y20=~PLC_16BIT[81].bits.bit0;
// 	Y21=~PLC_16BIT[81].bits.bit1;
// 	Y22=~PLC_16BIT[81].bits.bit2;
// 	Y23=~PLC_16BIT[81].bits.bit3;
//	Y24=~PLC_16BIT[81].bits.bit4;
//	Y25=~PLC_16BIT[81].bits.bit5;
//	Y26=~PLC_16BIT[81].bits.bit6;
//	Y27=~PLC_16BIT[81].bits.bit7;
//	Y30=~PLC_16BIT[81].bits.bit10;
//	Y31=~PLC_16BIT[81].bits.bit11;
//	Y32=~PLC_16BIT[81].bits.bit12;
//	Y33=~PLC_16BIT[81].bits.bit13;			
//  Y34=~PLC_16BIT[81].bits.bit14;
//	Y35=~PLC_16BIT[81].bits.bit15;
//	Y36=~PLC_16BIT[81].bits.bit16;
//	Y37=~PLC_16BIT[81].bits.bit17;	


	Y00=PLC_16BIT[80].bits.bit0;
	Y01=PLC_16BIT[80].bits.bit1;
	Y02=PLC_16BIT[80].bits.bit2;
	Y03=PLC_16BIT[80].bits.bit3;
	Y04=PLC_16BIT[80].bits.bit4;
	Y05=PLC_16BIT[80].bits.bit5;
	Y06=PLC_16BIT[80].bits.bit6;
	Y07=PLC_16BIT[80].bits.bit7;
	Y10=PLC_16BIT[80].bits.bit10;
	Y11=PLC_16BIT[80].bits.bit11;
	Y12=PLC_16BIT[80].bits.bit12;
	Y13=PLC_16BIT[80].bits.bit13;
	Y14=PLC_16BIT[80].bits.bit14;
	Y15=PLC_16BIT[80].bits.bit15;
// 	Y16=PLC_16BIT[80].bits.bit16;
// 	Y17=PLC_16BIT[80].bits.bit17;

// 	Y20=PLC_16BIT[81].bits.bit0;
// 	Y21=PLC_16BIT[81].bits.bit1;
// 	Y22=PLC_16BIT[81].bits.bit2;
// 	Y23=PLC_16BIT[81].bits.bit3;
//	Y24=PLC_16BIT[81].bits.bit4;
//	Y25=PLC_16BIT[81].bits.bit5;
//	Y26=PLC_16BIT[81].bits.bit6;
//	Y27=PLC_16BIT[81].bits.bit7;
//	Y30=PLC_16BIT[81].bits.bit10;
//	Y31=PLC_16BIT[81].bits.bit11;
//	Y32=PLC_16BIT[81].bits.bit12;
//	Y33=PLC_16BIT[81].bits.bit13;			
//  Y34=PLC_16BIT[81].bits.bit14;
//	Y35=PLC_16BIT[81].bits.bit15;
//	Y36=PLC_16BIT[81].bits.bit16;
//	Y37=PLC_16BIT[81].bits.bit17;		
}



void PLC_IO_config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);//RCC_APB2Periph_AFIO
	  Err_Lamp_OFF;	 //ADD
    PLC_Y_config(); 
    PLC_IO_Refresh();           //刷新Y输出()                              
    PLC_X_config();
	  PLC_Mode_config();
// 	PLC_EXTI15_10_config();	    // 小小晟,20160929屏蔽
//	PLC_EXTI9_5_config();	      // 小小晟,20160929屏蔽
}
