/********************************************************/
// CPU需要：STM32F103--RAM内存不小于64K	Flash内存不小于128K
// 本代码已在STM32F103RDT6、VET6测试通过
// 编辑日期：20150909
// editor by 小小晟
// 网店：shop182385147.taobao.com
/********************************************************/

#include "stm32f10x.h"
#include <stdio.h>
#include "PLC_IO.h"
#include "PLC_Dialogue.h"
#include "math.h"          //数学函数库 
#include "Gray.h"          //格雷码转换库
#include "PLC_CONF.H"

extern void RST_T_D_C_M_data(void);	 //ADD 20151214
extern unsigned char Y0P,Y1P;                //
extern unsigned short Plus_CMP0,Plus_CMP1;   //脉冲标志位
extern u8  X_DIY;	                           //滤波时间
extern u16 PLC_RUN_TIME;                     //扫描时间
extern void RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//时间修改程序
extern void PLC_IO_Refresh(void);            //IO刷新输出程序
extern u16 PLC_16BIT[12100];                 //PLC_RAM运行寄存器
extern void timer_enable(u16 timer_number);	 //此处在定时器2里面，用于开启定时器处理工
extern void timer_disble(u16 timer_number);	 //此处在定时器2里面     
static u8 PLC_ACC_BIT,PLC_MPS_BIT;           //程序执行专用(运算栈及分线栈)
static const u16 *PLC_Addr;                  //PLC程序指针
static const u16 *PLC_Err;                   //PLC出错步
static u8 T_number,C_number;                 //T&C地址缓存寄存器
static u16 T_value;                          //T比较缓存寄存器
static u16 C_value;                          //C比较缓存寄存器
static u32 mov_d_addr;                       //K?M&Y&S&X指令缓存
static const u16 *PLC_P_Addr[129];	         //方便调子程序取指针
static const u16 *p_save[129];               //调子程序时保存上一个执行点位
u8  Flag_bit=0xff,Transfer_bit,Transfer_bit1;//其实就是k的标志位  减小函数的量，减轻CPU负担     
u16 process[64];                             //调子程序时保存上一个子程序值
u32 trade;                                   //作用于加减法，减小函数的量，减轻CPU负担     
u16 Transfer=0;                              //作用多地传递和成批传递 减小函数的量，减轻CPU负担  
u8 edit_prog;                                //从新编程缓存寄存器
extern u8 Write_Pro_flag;
const unsigned char PLC_BIT_OR[]={0X01,0X02,0X04,0X08,0X10,0X20,0X40,0X80};
const unsigned char PLC_BIT_AND[]={0XFE,0XFD,0XFB,0XF7,0XEF,0XDF,0XBF,0X7F};
float_union FLOAT;
s32_union   u32data,u32data1;
u64_union   u64data,u64data2;

/***************************************************FOR**************************************************/
struct 
{
	const u16 *Addr[7];  //FOR 地址记录
	u16 cycle[7];        //当前循环的次数
	u16 count[7];        //目标循环的次数
	u8  point;           //for指向的点数	
} FOR_CMD;



/***************************************************STL**************************************************/
static u16 PLC_STL_Addr;	  //STL指令地址号
static u8  PLC_STL_Status;    //STL指令当前状态 0整套程序没有STL状态，程序1为STL有状态，2为STL停止状态 
static u8  PLC_STL_CMD;		  //STL标志
static u8  PLC_STL_Count;     //计数线圈数量
static u16 PLC_STL_Coil[256]; //线圈缓存寄存器
/********************************************************************************************************/
static u8 PLC_PL_BIT_TEST(u16 x){return((Step_Address[(x)/8] & PLC_BIT_OR[(x)%8])) ? (1) : (0);}
static u8 PLC_LD_BIT(u16 x){return((PLC_RAM8(RAM_ADDR+((x)/8)) & PLC_BIT_OR[(x)%8])) ? (1) : (0) ;}

static u8 PLC_LDP_TEST(void)	                        //查看是不上升沿
{ 
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)==off)//上升沿判断
	{ 
		   if(PLC_ACC_BIT&0X01)			                      //当前值判断
	     {
				 PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);   //
         return 1;
	     }
			 else return 0;
	 }
	 else
	 {
			if(!(PLC_ACC_BIT&0x01))						             //当前值判断
		  PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);    //
			return 0;
	 } 
}

/*********************************************
  函数功能：PLC代码错误处理程序
  err_id=01:指令出错(未识别指令)
  err_id=02:指令出错(暂不支持指令)
  err_id=10:数据出错(无法识别数据类型)
  err_id=11:数据出错(数据读取地址超出)
  err_id=12:数据出错(变址Z地址未知)
  err_id=13:数据出错(变址Z地址超出)
  err_id=20:CJ指令地址出错
  D8061,M8061=PC硬件错误
  D8063,M8063=链接,通信错误
  D8064,M8064=参数错误
  D8065,M8065=语法错误
  D8066,M8066=回路错误
  D8067,M8067=运算错误
  D8068,M8068=运算错误锁存
***********************************************/
void PLC_PROG_ERROR(u16 err,u16 err_id)
{
// PLC_BIT_ON(err);                              //出错标志  小小晟 屏蔽
 D8012=0;	                                     //扫描时间
 if (D8068==0)D8067=err_id;                      //语法错误
 if (D8068==0)D8068=(PLC_Err-(u16*)(0x800605D)); //保存出错PC步
 D8069=D8068;
}

static void LD(u16 start_addr)	 //起始地址，加元件编号值
{ 
	 if(PLC_STL_Status == 1)                       //为STL状态区  全局步进
	 {  
		PLC_ACC_BIT<<=1;
		if(PLC_BIT_TEST(start_addr)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT |=0x01;
	 }	
   else
    {  
		PLC_ACC_BIT<<=1;
		if(PLC_BIT_TEST(start_addr)) 
		PLC_ACC_BIT |=0x01;
	}		 		
}

static void LDI(u16 start_addr)
{ 
	if(PLC_STL_Status == 1)                          //为STL状态区  全局步进
	{   
		PLC_ACC_BIT<<=1;
		if((!(PLC_BIT_TEST(start_addr)))&&(PLC_BIT_TEST(PLC_STL_Addr)))
		PLC_ACC_BIT |=0x01;
	}
	else	
	{		
		PLC_ACC_BIT<<=1;
		if(PLC_BIT_TEST(start_addr));
		else		
		PLC_ACC_BIT |=0x01;
	}
}

void AND(u16 start_addr)
{ 
	if((PLC_BIT_TEST(start_addr))&&(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE; 
}

static void ANI(u16 start_addr)
{ 
	if((!(PLC_BIT_TEST(start_addr)))&&(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE; 
}

static void OR(u16 start_addr)
{ 
	if((PLC_BIT_TEST(start_addr))||(PLC_ACC_BIT&0X01))
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE; 
}

static void ORI(u16 start_addr)
{ 
	if((!(PLC_BIT_TEST(start_addr)))||(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE; 
}

void OUT(u16 start_addr)
{	
   if (PLC_STL_CMD == 1)                 //判断是不是进入步进模式
	 {
      if (PLC_STL_Status == 1)           //是不是打开STL模式
			{
				 if(start_addr < 0X000A)//判断开始步进 S000-S009
			   {
		             if((PLC_ACC_BIT&0x01)==0x01)
		             {	
							   PLC_BIT_OFF(PLC_STL_Addr); //OFF			      
							   PLC_BIT_ON(start_addr);    //ON						 
				     }
			   }
				 else 
				 {
					   if(PLC_BIT_TEST(PLC_STL_Addr))
					   {						 
				        if((PLC_ACC_BIT&0x01)==0x01)             
								{ 
									PLC_BIT_ON(start_addr);       //ON
								  PLC_STL_Coil[PLC_STL_Count++]=start_addr;//记录步进中ON线圈地址 位下个步进成立清除用
								}       
				        else 
 								  PLC_BIT_OFF(start_addr);      //OFF 
						 }

				 }
     }
     else
		 {
				 if(start_addr < 0X000A)//判断开始步进 S000-S009
			   {
				     if(PLC_ACC_BIT & 0x01)
				     {
				       PLC_BIT_ON(start_addr); //ON
				     }
				 }
				 else
				 {
				    if(PLC_ACC_BIT&0x01)
				       PLC_BIT_ON(start_addr);         //ON 
						else 
						   PLC_BIT_OFF(start_addr);      //OFF 
				 }
     }
	 }
	 else
	 {
       if(PLC_ACC_BIT&0X01)
       PLC_BIT_ON(start_addr);    //ON
	     else
	     PLC_BIT_OFF(start_addr);   //OFF 
   }
}


static void BIT_SET(u16 start_addr)//位设置
{ 
	u8 temp;
	if(PLC_ACC_BIT&0x01)
	{
		if (PLC_STL_Status == 1)       //为STL状态区
		{
			for(temp=0;temp<=PLC_STL_Count;temp++)
			PLC_BIT_OFF(PLC_STL_Coil[temp]);    //清除上次ON线圈状态
			
			PLC_BIT_OFF(PLC_STL_Addr); //OFF
			PLC_BIT_ON(start_addr);    //ON
			PLC_STL_Count=0;           //清除上次记录ON线圈数量
		}
		else PLC_BIT_ON(start_addr);   //0N
	}
}


static void RST(u16 start_addr)//复位位
{ 
  if((PLC_ACC_BIT&0X01)==0X01)
  PLC_BIT_OFF(start_addr);     //OFF 
}

static void RET(void)
{  
  PLC_STL_Status =0;               //退出步进模式 让程序进入梯形图
}

void STL(u16 start_addr)	      //步进 模式
{
	PLC_STL_CMD = 1;            //全局程序启用步进标志
	PLC_STL_Status = 1;         //启动步进模式
	PLC_STL_Addr = start_addr;  //记录步进地址
	PLC_ACC_BIT<<=1;
	if(PLC_BIT_TEST(PLC_STL_Addr))     
	PLC_ACC_BIT |=0x01;
}

// 小小晟，20160926优化
static void other_function(u8 process_addr)
{
	 switch(process_addr)
   { 
		case 0xF8: //块串联 ANB 
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT >> 1)   & ((PLC_ACC_BIT & 0x01)|0xFE);           
			break;  
		}
		case 0xF9: //块并联 ORB
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT >> 1)   | (PLC_ACC_BIT & 0x01);                  
			break;  
	  }
		case 0xFA: //进栈   MPS
    {			
			PLC_MPS_BIT = (PLC_MPS_BIT << 1)   | (PLC_ACC_BIT & 0x01);                  
			break;  
		}
		case 0xFB: //读栈   MRD
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT & 0xfe) | (PLC_MPS_BIT & 0x01);                  
			break;	
		}
		case 0xFC: //出栈   MPP
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT & 0xfe) | (PLC_MPS_BIT & 0x01),PLC_MPS_BIT >>= 1;
			break;  
		}
		case 0xFD: // 取反   INV  
		{
			PLC_ACC_BIT = (PLC_ACC_BIT & 0xfe) | (~PLC_ACC_BIT & 0x01);                 
			break;
		}
		case 0xFF: //取反   POP  
    {			
			break;  
		}
		default:
		{
			PLC_PROG_ERROR(M8064,02);                                                        
		  break;
		}
   }
}


static void LPS(void)              //M1536~M3071位LPS指令函数
{ 
	 if(PLC_ACC_BIT&0x01)
	 {
		  if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)==0)
		  {
			   PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);
			   PLC_BIT_ON((0x2fff&*PLC_Addr));	 
		  }
			else{PLC_BIT_OFF((0x2fff&*PLC_Addr));}
	 }
	 else{PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);}
	 PLC_Addr++;
}

static void LPF(void)              //M1536~M3071位LPS指令函数
{ 
   if(PLC_ACC_BIT&0x01)
	 {
		if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)==0)
		{PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);}
	 }
	 else
	 {
		   if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)) 
		   {
			   PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);	 
		       PLC_BIT_ON((0x2fff&*PLC_Addr));
		   }	 
		   else{PLC_BIT_OFF((0x2fff&*PLC_Addr));} 
	 }
	 PLC_Addr++;
}


static void RESET_T(u8 process_addr) //定时器复位
{  
	if(PLC_ACC_BIT&0x01) 			         //当前值是否有效
	{
		PLC_BIT_OFF(0x600+process_addr); //溢出线圈
		PLC_BIT_OFF(0x2600+process_addr);//使能线圈
		PLC_BIT_ON(0x2300+process_addr); //复位线圈
		PLC_16BIT[0x0800+process_addr]=0;//实际计数辶
	}
	else
	PLC_BIT_OFF(0x2300+process_addr);   //复位线圈
}

static void RESET_C(u8 process_addr) 	              //定时器复位
{ 
	static u16 *p_data;
	if((PLC_ACC_BIT&0x01)==0x01) 		                  //当前值是否有效
	{ 
		if((process_addr>=0XC8)&&(process_addr<=0XFF))
		{
			p_data=PLC_16BIT+0x0500+process_addr;         //指向值地址
			*p_data=0;                                    //清零地址 高位
			p_data+=1;				                             //因为是32位
			*p_data=0;                                    //清零地址 低位
			PLC_BIT_OFF(0x00E0+process_addr);             //指向溢出线圈并清溢出线圈 
		}
		else
		{
			p_data=PLC_16BIT+0x0500+process_addr;           //指向值地址
			*p_data=0;							                        //清零地址
			PLC_BIT_OFF(0x00E0+process_addr);              //指向溢出线圈并清溢出线圈               
		}
	}
	OUT(0X3700+process_addr);
}

static void RST_T_C(void)                            //所有T_C位RST指令函数
{  
	switch(*PLC_Addr/0x100)
	{
		case 0x86: RESET_T(*PLC_Addr),PLC_Addr++;break;//复位T
		case 0x8E: RESET_C(*PLC_Addr),PLC_Addr++;break;//复位C
	}
}

 
static void MOV_TO_K_H(u8 i,u32 data,u8 addr)//进行 MOV ?? K?X&Y&S&M 地址计算
{  
	u8 LL_BIT;                         //需要向左移动多少位用寄存器
	u16 JOB_ADDR;
	int64_t MOV_DATA_64BIT,MOV_DATA_64BIT_BACKUP,MOV_DATA_BACKUP1;  //移动32位数据
	mov_d_addr|=addr<<8; 
	mov_d_addr+=Transfer;                      //成批传递和多点时 Transfer会有数据平时都是0
	LL_BIT=mov_d_addr%0x20;						         //需要移动多少位
	JOB_ADDR=(mov_d_addr/0x20)*4;              //所在的起始地址  
	switch(i)							 
	{            //	  移动位数　需要的数据做好处理					  将数据移动需要的位数			 后面要将数据取反用
		case 0x82: MOV_DATA_64BIT_BACKUP=data&0X0000000F,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X0000000F<<LL_BIT); break;//传送K1大小
		case 0x84: MOV_DATA_64BIT_BACKUP=data&0X000000FF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X000000FF<<LL_BIT); break;//传送K2大小
		case 0x86: MOV_DATA_64BIT_BACKUP=data&0X00000FFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X00000FFF<<LL_BIT); break;//传送K3大小
		case 0x88: MOV_DATA_64BIT_BACKUP=data&0X0000FFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X0000FFFF<<LL_BIT); break;//传送K4大小
		case 0x8A: MOV_DATA_64BIT_BACKUP=data&0X000FFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X000FFFFF<<LL_BIT); break;//传送K5大小
		case 0x8C: MOV_DATA_64BIT_BACKUP=data&0X00FFFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X00FFFFFF<<LL_BIT); break;//传送K6大小
		case 0x8E: MOV_DATA_64BIT_BACKUP=data&0X0FFFFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X0FFFFFFF<<LL_BIT); break;//传送K7大小
		case 0x90: MOV_DATA_64BIT_BACKUP=data&0XFFFFFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0XFFFFFFFF<<LL_BIT); break;//传送K8大小
		default:   PLC_Addr+=3;                           break;  //遇到不支持的命令
	}                                          
	MOV_DATA_64BIT=PLC_RAM64(RAM_ADDR+JOB_ADDR);
	MOV_DATA_64BIT&=MOV_DATA_BACKUP1;             //将需要传送的位置清空 
	MOV_DATA_64BIT|=MOV_DATA_64BIT_BACKUP;        //传入需要到的位置   
	PLC_RAM64(RAM_ADDR+JOB_ADDR)=MOV_DATA_64BIT;  //将数据传到目标位置
}
 

static signed int MOV_K(u8 Addr)	              //进行K?X&Y&S&M计算
{ 
	static u16 LL_BIT,JOB_ADDR;                   //需要向左移动多少位用寄存器
	static uint64_t MOV_DATA_64BIT;               //移动64位数据
	mov_d_addr|=(Addr<<8);                    
	mov_d_addr+=Transfer;                         //成批传递和多点时 Transfer会有数据平时都是0
	LL_BIT=mov_d_addr%0x20;						            //需要移动多少位
	JOB_ADDR=(mov_d_addr/0x20)*4;                 //所在的起始地址                 
	MOV_DATA_64BIT=PLC_RAM64(RAM_ADDR+JOB_ADDR),
	MOV_DATA_64BIT>>=LL_BIT; 									 
	return  (signed int)MOV_DATA_64BIT;
}

//计算出高位地址
u16 D_C_T_addr(u8 l_value)
{ 
	static u16 temp; 
	switch(*PLC_Addr/0x100)
	{
		case 0x80: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0700+temp/2,PLC_Addr++;      break;//大于等于D1000
		case 0x82: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0800+temp/2,PLC_Addr++;      break;//算出T的地址，将地址变为值
		case 0x84: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0500+temp/2,PLC_Addr++;      break;//算出C的地址，将地址变为值
		case 0x86: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x1000+temp/2,PLC_Addr++;      break;//算出D的地址，将地址变为值
		case 0x88: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x1000+temp/2+1000,PLC_Addr++; break;//大于等于D1000
	}
	return temp;
}

//=======================================================================================================
// 函数名称:  static u16 addr_value(void)
// 功能描述： 计算PLC地址或k的实数
// 输　入:  void      
// 输　出:  地址或数据     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟			  
// 日　期:  2015年6月2日
// 备  注:  
//-------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 优化程序数量测试中部分指令测试  优化后返回地址 k是数据
//-------------------------------------------------------------------------------------------------------
//=======================================================================================================
static u16 addr_value(void)                                        
{  
	static u8 temp;static u16 temp1;
	switch(*PLC_Addr/0x100)
	{
		case 0x84: temp=*PLC_Addr,PLC_Addr++,temp1=*PLC_Addr<<8|temp,PLC_Addr++,Flag_bit=0;break;//进行如 K4M0 之类的传送                                              break;//进行C的地址
		case 0x86: temp=*PLC_Addr,PLC_Addr++;temp1=D_C_T_addr(temp);                       break;//算出D、C、T的地址          
	}
	return temp1;
}



//=======================================================================================================
// 函数名称:  static u32 addr_value_prog(void)   
// 功能描述： 计算PLC地址或k的实数
// 输　入:  void      
// 输　出:  地址或数据     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月23日
// 备  注:  
//-------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//=======================================================================================================
static u32 addr_value_prog(void)                                      
{  
	static u32 temp; 
	u16 Type_F,temp2,Data1,Data2;
	Data1=*PLC_Addr;PLC_Addr++;Data2=*PLC_Addr;
	temp2=Type_F=0;
	Type_F   = (Data1 & 0xff00);
	Type_F  |= (Data2 >> 8);
	
	temp2  = (Data2 << 8);
	temp2 |=mov_d_addr=(u8)Data1;
	
	if(Type_F == 0x8680)      temp=RAM_D8000_ADDR+temp2,                      PLC_Addr++;//算出D的地址 D8000
	else if(Type_F == 0x8682) temp=RAM_T_ADDR+temp2,                      PLC_Addr++;//算出T的地址 
	else if(Type_F == 0x8684) temp=RAM_C_ADDR+temp2,                      PLC_Addr++;//算出C的地址
	else if(Type_F == 0x8686) temp=RAM_D_ADDR+temp2,                      PLC_Addr++;//算出D的地址 
	else if(Type_F == 0x8688) temp=RAM_D1000_ADDR+temp2,                      PLC_Addr++;//大于等于D1000
	else if(Type_F == 0x8482) temp=MOV_K(*PLC_Addr)&0X0000000F,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x8484) temp=MOV_K(*PLC_Addr)&0X000000FF,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x8486) temp=MOV_K(*PLC_Addr)&0X00000FFF,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x8488) temp=MOV_K(*PLC_Addr)&0X0000FFFF,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x848A) temp=MOV_K(*PLC_Addr)&0X000FFFFF,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x848C) temp=MOV_K(*PLC_Addr)&0X00FFFFFF,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x848E) temp=MOV_K(*PLC_Addr)&0X0FFFFFFF,Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送    
	else if(Type_F == 0x8490) temp=MOV_K(*PLC_Addr),           Flag_bit=0,PLC_Addr++;//进行如 K4M0 之类的传送  
	return temp;
}

unsigned short V0_V3(u16 temp1)
{
	u8 temp=PLC_v_z_addr(temp1);
	if(temp==0)       return D8029;
	else if(temp==1)  return D8183;
	else if(temp==2)  return D8185;
	else if(temp==3)  return D8187;
	else	return 0;
}

unsigned short V4_V7(u16 temp1)
{
	u8 temp=PLC_v_z_addr(temp1);
	if(temp==0)       return D8189;
  else if(temp==1)  return D8191;
  else if(temp==2)  return D8193;
  else if(temp==3)  return D8195;
  else	return 0;
}

unsigned short Z0_Z3(u16 temp1)
{
	u8 temp=PLC_v_z_addr(temp1);
	if(temp==0)       return D8028;
  else if(temp==1)  return D8182;
  else if(temp==2)  return D8184;
  else if(temp==3)  return D8186;
  else	return 0;
}

unsigned short Z4_Z7(u16 temp1)
{
	u8 temp=PLC_v_z_addr(temp1);
	if(temp==0)       return D8188;
  else if(temp==1)  return D8190;
  else if(temp==2)  return D8192;
  else if(temp==3)  return D8194;
  else	return 0;
}

unsigned int DZ0_Z3(u16 temp1)
{
	u8 temp=PLC_v_z_addr(temp1);
	if(temp==0)       return D8028+D8029*0X10000;
  else if(temp==1)  return D8182+D8183*0X10000;
  else if(temp==2)  return D8184+D8184*0X10000;
  else if(temp==3)  return D8186+D8185*0X10000;
  else	return 0;
}

unsigned short DZ4_Z7(u16 temp1)
{
	u8 temp=PLC_v_z_addr(temp1);
	if(temp==0)       return D8188+D8189*0X10000;
  else if(temp==1)  return D8190+D8191*0X10000;
  else if(temp==2)  return D8192+D8193*0X10000;
  else if(temp==3)  return D8194+D8193*0X10000;
  else	return 0;
}
//=======================================================================================================
// 函数名称:  static void target(void)
// 功能描述： 加减法“与”“或”“异或”共用赋值函数	如DMOV、DADD、DSUB等指令的结果传递出去
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月23日
// 备  注:  
//=======================================================================================================
static void D_target(void)                                      
{  
	 u16 Type_F,temp2,Data1,Data2;
	 Data1=*PLC_Addr;PLC_Addr++;Data2=*PLC_Addr;
	 temp2=Type_F=0;
	 Type_F   = (Data1 & 0xff00);
	 Type_F  |= (Data2 >> 8);
/************************************/	
	 temp2  = (Data2 << 8);
	 temp2 |=mov_d_addr=(u8)Data1;
/************************************/	
	switch(Type_F)
	{
		case 0x8482: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x8484: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x8486: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x8488: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x848A: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x848C: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x848E: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case 0x8490: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送     
		
		case 0x8680: PLC_RAM32(RAM_D8000_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //算出D的地址 D8000
		case 0x8682: PLC_RAM32(RAM_T_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //算出T的地址 
		case 0x8684: PLC_RAM32(RAM_C_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //算出C的地址
		case 0x8686: PLC_RAM32(RAM_D_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //算出D的地址 
		case 0x8688: PLC_RAM32(RAM_D1000_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //大于等于D1000
		/************************************************K1M0寄存器"Z"*******************************************************************/
		case 0xA482: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0的V0-V3	
		case 0xA483: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K2M0寄存器"Z"*******************************************************************/
		case 0xA484: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0的V0-V3	
		case 0xA485: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K3M0寄存器"Z"*******************************************************************/
		case 0xA486: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0的V0-V3	
		case 0xA487: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K4M0寄存器"Z"*******************************************************************/
		case 0xA488: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V0-V3
		case 0xA489: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V4-V7		
		/************************************************K5M0寄存器"Z"*******************************************************************/
		case 0xA48A: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K5M0的V0-V3	
		case 0xA48B: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K5M0的V4-V7		
		/************************************************K6M0寄存器"Z"*******************************************************************/
		case 0xA48C: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K6M0的V0-V3	
		case 0xA48D: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K6M0的V4-V7		
		/************************************************K7M0寄存器"Z"*******************************************************************/
		case 0xA48E: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K7M0的V0-V3	
		case 0xA48F: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K7M0的V4-V7		
		/************************************************K8M0寄存器"Z"*******************************************************************/
		case 0xA490: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//k8M0的V0-V4	
		case 0xA491: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K8M0的V4-V7			
		/************************************************T寄存器"Z"*******************************************************************/
		case 0xA682: PLC_RAM32(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出T的地址，的Z0-Z3
		case 0xA683: PLC_RAM32(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出T的地址，的Z4-Z7
		/************************************************C寄存器"Z"*******************************************************************/
		case 0xA684: PLC_RAM32(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的Z0-Z3
		case 0xA685: PLC_RAM32(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的Z4-Z7
		/************************************************D寄存器"Z"*******************************************************************/
		case 0xA686: PLC_RAM32(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出D的地址，的Z0-Z3
		case 0xA687: PLC_RAM32(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出D的地址，的Z4-Z7
		case 0xA688: PLC_RAM32(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//大于等于D1000的Z0-Z3
		case 0xA689: PLC_RAM32(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//大于等于D1000的Z4-Z7
 }
	 PLC_Addr+=2;
}

//=======================================================================================================
// 函数名称:  static void target(void)
// 功能描述： 加减法“与”“或”“异或”共用赋值函数	如MOV、ADD、SUB等指令的结果传递出去
// 输　入:  void      
// 输　出:  void
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月23日
// 备  注:  
//=======================================================================================================
static void target(void)                                      
{  
	 u16 Type_F,temp2,Data1,Data2;
	 Data1=*PLC_Addr;PLC_Addr++;Data2=*PLC_Addr;
	 temp2=Type_F=0;
	 Type_F   = (Data1 & 0xff00);
	 Type_F  |= (Data2 >> 8);
/************************************/	
	 temp2  = (Data2 << 8);
	 temp2 |=mov_d_addr=(u8)Data1;
/************************************/	
	switch(Type_F)
	{
		case  0x8482: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case  0x8484: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case  0x8486: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		case  0x8488: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //进行如 K4M0 之类的传送    
		
		case  0x8680: PLC_RAM16(RAM_D8000_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //算出D的地址 D8000
		case  0x8682: PLC_RAM16(RAM_T_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //算出T的地址 
		case  0x8684: PLC_RAM16(RAM_C_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //算出C的地址
		case  0x8686: PLC_RAM16(RAM_D_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //算出D的地址 
		case  0x8688: PLC_RAM16(RAM_D1000_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //大于等于D1000
		/************************************************K1M0寄存器"V"*******************************************************************/
		case  0x9482: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0的V0-V3	
		case  0x9483: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K2M0寄存器"V"*******************************************************************/
		case  0x9484: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0的V0-V3	
		case  0x9485: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K3M0寄存器"V"*******************************************************************/
		case  0x9486: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0的V0-V3	
		case  0x9487: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K4M0寄存器"V"*******************************************************************/
		case  0x9488: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V0-V3	
		case  0x9489: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V4-V7						
		/************************************************T寄存器"V"*******************************************************************/
		case  0x9682: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出T的地址，的V0-V3
		case  0x9683: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出T的地址，的V4-V7
		/************************************************C寄存器"V"*******************************************************************/
		case  0x9684: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的V0-V3
		case  0x9685: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的V4-V7
		/************************************************D寄存器"V"*******************************************************************/
		case  0x9686: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出D的地址，的V0-V3
		case  0x9687: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出D的地址，的V4-V7
		case  0x9688: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//大于等于D1000的V0-V3
		case  0x9689: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//大于等于D1000的V4-V7
		
		/************************************************K1M0寄存器"Z"*******************************************************************/
		case  0xA482: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0的V0-V3	
		case  0xA483: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K2M0寄存器"Z"*******************************************************************/
		case  0xA484: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0的V0-V3	
		case  0xA485: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K3M0寄存器"Z"*******************************************************************/
		case  0xA486: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0的V0-V3	
		case  0xA487: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K4M0寄存器"Z"*******************************************************************/
		case  0xA488: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V0-V3
		case  0xA489: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V4-V7			
		/************************************************T寄存器"Z"*******************************************************************/
		case  0xA682: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的Z0-Z3
		case  0xA683: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的Z4-Z7
		/************************************************C寄存器"Z"*******************************************************************/
		case  0xA684: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的Z0-Z3
		case  0xA685: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出C的地址，的Z4-Z7
		/************************************************D寄存器"Z"*******************************************************************/
		case  0xA686: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出D的地址，的Z0-Z3
		case  0xA687: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//算出D的地址，的Z4-Z7
		case  0xA688: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//大于等于D1000的Z0-Z3
		case  0xA689: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//大于等于D1000的Z4-Z7
    }
}


//=======================================================================================================
// 函数名称:  static u16  cos_value(void)	
// 功能描述： 
// 输　入:  void      
// 输　出:  输出16位数据
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月19日
// 备  注:  
//-------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//-------------------------------------------------------------------------------------------------------
//=======================================================================================================
static s16 cos_value()                                      
{  
  static s16 temp; 
	 u16 Type_F,temp2,Data1,Data2;
	 Data1=*PLC_Addr;PLC_Addr++;Data2=*PLC_Addr;
	 temp2=Type_F=0;
	 Type_F   = (Data1 & 0xff00);
	 Type_F  |= (Data2 >> 8);
/************************************/	
	 temp2  = (Data2 << 8);
	 temp2 |=mov_d_addr=(u8)Data1;
/************************************/	
	switch(Type_F)
	{
		case  0x8080: temp=temp2,                                      PLC_Addr++;break;  //算出K值
		case  0x8280: temp=temp2,                                      PLC_Addr++;break;  //算出H值
		case  0x8482: temp=MOV_K(*PLC_Addr)&0X0000000F,Transfer_bit1=1,PLC_Addr++;break;  //进行如 K4M0 之类的传送    
		case  0x8484: temp=MOV_K(*PLC_Addr)&0X000000FF,Transfer_bit1=1,PLC_Addr++;break;  //进行如 K4M0 之类的传送    
		case  0x8486: temp=MOV_K(*PLC_Addr)&0X00000FFF,Transfer_bit1=1,PLC_Addr++;break;  //进行如 K4M0 之类的传送    
		case  0x8488: temp=MOV_K(*PLC_Addr)&0X0000FFFF,Transfer_bit1=1,PLC_Addr++;break;  //进行如 K4M0 之类的传送    	
		case  0x8680: temp=PLC_RAM16(RAM_D8000_ADDR+temp2+Transfer),PLC_Addr++;break;         //算出D的地址 D8000
		case  0x8682: temp=PLC_RAM16(RAM_D_ADDR+temp2+Transfer),PLC_Addr++;break;         //算出T的地址 
		case  0x8684: temp=PLC_RAM16(RAM_C_ADDR+temp2+Transfer),PLC_Addr++;break;         //算出C的地址
		case  0x8686: temp=PLC_RAM16(RAM_D_ADDR+temp2+Transfer),PLC_Addr++;break;         //算出D的地址 
		case  0x8688: temp=PLC_RAM16(RAM_D1000_ADDR+temp2+Transfer),PLC_Addr++;break;         //大于等于D1000
		 	/************************************************K寄存器"V"*******************************************************************/
		case  0x9080: temp=temp2+D8029,PLC_Addr++;break;//算出K的地址，的V0
		case  0x9081: temp=temp2+D8183,PLC_Addr++;break;//算出K的地址，的V1
		case  0x9082: temp=temp2+D8185,PLC_Addr++;break;//算出K的地址，的V2
		case  0x9083: temp=temp2+D8187,PLC_Addr++;break;//算出K的地址，的V3
		case  0x9084: temp=temp2+D8189,PLC_Addr++;break;//算出K的地址，的V4
		case  0x9085: temp=temp2+D8191,PLC_Addr++;break;//算出K的地址，的V5
		case  0x9086: temp=temp2+D8193,PLC_Addr++;break;//算出K的地址，的V6
		case  0x9087: temp=temp2+D8195,PLC_Addr++;break;//算出K的地址，的V7
		case  0x9280: temp=temp2+D8029,PLC_Addr++;break;//算出H的地址，的V0
		case  0x9281: temp=temp2+D8183,PLC_Addr++;break;//算出H的地址，的V1
		case  0x9282: temp=temp2+D8185,PLC_Addr++;break;//算出H的地址，的V2
		case  0x9283: temp=temp2+D8187,PLC_Addr++;break;//算出H的地址，的V3
		case  0x9284: temp=temp2+D8189,PLC_Addr++;break;//算出H的地址，的V4
		case  0x9285: temp=temp2+D8191,PLC_Addr++;break;//算出H的地址，的V5
		case  0x9286: temp=temp2+D8193,PLC_Addr++;break;//算出H的地址，的V6
		case  0x9287: temp=temp2+D8195,PLC_Addr++;break;//算出H的地址，的V7 	
		/************************************************K1M0寄存器"V"*******************************************************************/
		case  0x9482: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0的V0-V3	
		case  0x9483: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K2M0寄存器"V"*******************************************************************/
		case  0x9484: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0的V0-V3	
		case  0x9485: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K3M0寄存器"V"*******************************************************************/
		case  0x9486: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0的V0-V3	
		case  0x9487: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K4M0寄存器"V"*******************************************************************/
		case  0x9488: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0的V0-V3	
		case  0x9489: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0的V4-V7						
		/************************************************T寄存器"V"*******************************************************************/
		case  0x9682: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//算出T的地址，的V0-V3
		case  0x9683: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//算出T的地址，的V4-V7
		/************************************************C寄存器"V"*******************************************************************/
		case  0x9684: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//算出C的地址，的V0-V3
		case  0x9685: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//算出C的地址，的V4-V7
		/************************************************D寄存器"V"*******************************************************************/
		case  0x9686: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//算出D的地址，的V0-V3
		case  0x9687: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//算出D的地址，的V4-V7
		case  0x9688: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//大于等于D1000的V0-V3
		case  0x9689: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//大于等于D1000的V4-V7
	
		case  0xA080: temp=temp2+D8028,PLC_Addr++;break;//算出K的地址，的Z0
		case  0xA081: temp=temp2+D8182,PLC_Addr++;break;//算出K的地址，的Z1
		case  0xA082: temp=temp2+D8184,PLC_Addr++;break;//算出K的地址，的Z2
		case  0xA083: temp=temp2+D8186,PLC_Addr++;break;//算出K的地址，的Z3
		case  0xA084: temp=temp2+D8188,PLC_Addr++;break;//算出K的地址，的Z4
		case  0xA085: temp=temp2+D8190,PLC_Addr++;break;//算出K的地址，的Z5
		case  0xA086: temp=temp2+D8192,PLC_Addr++;break;//算出K的地址，的Z6
		case  0xA087: temp=temp2+D8194,PLC_Addr++;break;//算出K的地址，的Z7 	
		case  0xA280: temp=temp2+D8028,PLC_Addr++;break;//算出H的地址，的Z0
		case  0xA281: temp=temp2+D8182,PLC_Addr++;break;//算出H的地址，的Z1
		case  0xA282: temp=temp2+D8184,PLC_Addr++;break;//算出H的地址，的Z2
		case  0xA283: temp=temp2+D8186,PLC_Addr++;break;//算出H的地址，的Z3
		case  0xA284: temp=temp2+D8188,PLC_Addr++;break;//算出H的地址，的Z4
		case  0xA285: temp=temp2+D8190,PLC_Addr++;break;//算出H的地址，的Z5
		case  0xA286: temp=temp2+D8192,PLC_Addr++;break;//算出H的地址，的Z6
		case  0xA287: temp=temp2+D8194,PLC_Addr++;break;//算出H的地址，的Z7 	
		/************************************************K1M0寄存器"Z"*******************************************************************/
		case  0xA482: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0的V0-V3	
		case  0xA483: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K2M0寄存器"Z"*******************************************************************/
		case  0xA484: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0的V0-V3	
		case  0xA485: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K3M0寄存器"Z"*******************************************************************/
		case  0xA486: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0的V0-V3	
		case  0xA487: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K4M0寄存器"Z"*******************************************************************/
		case  0xA488: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0的V0-V3
		case  0xA489: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0的V4-V7		
		case  0xA682: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//算出T的地址，的Z0-Z3
		case  0xA683: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//算出T的地址，的Z4-Z7
		/************************************************C寄存器"Z"*******************************************************************/
		case  0xA684: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//算出C的地址，的Z0-Z3
		case  0xA685: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//算出C的地址，的Z4-Z7
		/************************************************D寄存器"Z"*******************************************************************/
		case  0xA686: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//算出D的地址，的Z0-Z3
		case  0xA687: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//算出D的地址，的Z4-Z7
		case  0xA688: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//大于等于D1000的Z0-Z3
		case  0xA689: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//大于等于D1000的Z4-Z7
	}	
	return temp;
}
//=======================================================================================================
// 函数名称:  static u32 cos_u32_value(void) 
// 功能描述： 
// 输　入:  void      
// 输　出:  输出32位数据
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月19日
// 备  注:  
//-------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//-------------------------------------------------------------------------------------------------------
//=======================================================================================================
#define  D_data  u32data.data 
static s32 cos_u32_value(void)                                      
{   
	 u16 Type_F,temp2,Data1,Data2;
	 unsigned short temp;
	 Data1=*PLC_Addr;
	 PLC_Addr++;
	 Data2=*PLC_Addr;
	 temp2=Type_F=0;
	 Type_F   = (Data1 & 0xff00);
	 Type_F  |= (Data2 >> 8);
/************************************/	
	 temp2  = (Data2 << 8);
	 temp2 |=mov_d_addr=(u8)Data1;
/************************************/
	switch(Type_F)
	{		
		case  0x8080: u32data.data1[0]=temp2,PLC_Addr++,u32data.data1[1]=cos_value(),PLC_Addr-=2;break;//算出K值
		case  0x8280: u32data.data1[0]=temp2,PLC_Addr++,u32data.data1[1]=cos_value(),PLC_Addr-=2;break;//算出H值
		case  0x8482: D_data=MOV_K(*PLC_Addr)&0X0000000F,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x8484: D_data=MOV_K(*PLC_Addr)&0X000000FF,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x8486: D_data=MOV_K(*PLC_Addr)&0X00000FFF,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x8488: D_data=MOV_K(*PLC_Addr)&0X0000FFFF,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x848A: D_data=MOV_K(*PLC_Addr)&0X000FFFFF,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x848C: D_data=MOV_K(*PLC_Addr)&0X00FFFFFF,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x848E: D_data=MOV_K(*PLC_Addr)&0X0FFFFFFF,Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送    
		case  0x8490: D_data=MOV_K(*PLC_Addr),           Transfer_bit1=1,PLC_Addr++;break;//进行如 K4M0 之类的传送   
			
		case  0x8680: D_data=PLC_RAM32(RAM_D8000_ADDR+temp2),PLC_Addr++;break;                 //算出D的地址 D8000
		case  0x8682: D_data=PLC_RAM32(RAM_T_ADDR+temp2),PLC_Addr++;break;                 //算出T的地址 
		case  0x8684: D_data=PLC_RAM32(RAM_C_ADDR+temp2),PLC_Addr++;break;                 //算出C的地址
		case  0x8686: D_data=PLC_RAM32(RAM_D_ADDR+temp2),PLC_Addr++;break;                 //算出D的地址 
		case  0x8688: D_data=PLC_RAM32(RAM_D1000_ADDR+temp2),PLC_Addr++;break;                 //大于等于D1000
		case  0xA080: u32data.data1[0]=temp2+D8028,PLC_Addr++,u32data.data1[1]=cos_value()+D8029,PLC_Addr-=2;break;//算出K的地址，的Z0
		case  0xA081: u32data.data1[0]=temp2+D8182,PLC_Addr++,u32data.data1[1]=cos_value()+D8183,PLC_Addr-=2;break;//算出K的地址，的Z1
		case  0xA082: u32data.data1[0]=temp2+D8184,PLC_Addr++,u32data.data1[1]=cos_value()+D8185,PLC_Addr-=2;break;//算出K的地址，的Z2
		case  0xA083: u32data.data1[0]=temp2+D8186,PLC_Addr++,u32data.data1[1]=cos_value()+D8187,PLC_Addr-=2;break;//算出K的地址，的Z3
		case  0xA084: u32data.data1[0]=temp2+D8188,PLC_Addr++,u32data.data1[1]=cos_value()+D8189,PLC_Addr-=2;break;//算出K的地址，的Z4
		case  0xA085: u32data.data1[0]=temp2+D8190,PLC_Addr++,u32data.data1[1]=cos_value()+D8191,PLC_Addr-=2;break;//算出K的地址，的Z5
		case  0xA086: u32data.data1[0]=temp2+D8192,PLC_Addr++,u32data.data1[1]=cos_value()+D8193,PLC_Addr-=2;break;//算出K的地址，的Z6
		case  0xA087: u32data.data1[0]=temp2+D8194,PLC_Addr++,u32data.data1[1]=cos_value()+D8195,PLC_Addr-=2;break;//算出K的地址，的Z7 
		case  0xA280: u32data.data1[0]=temp2+D8028,PLC_Addr++,u32data.data1[1]=cos_value()+D8029,PLC_Addr-=2;break;//算出H的地址，的Z0
		case  0xA281: u32data.data1[0]=temp2+D8182,PLC_Addr++,u32data.data1[1]=cos_value()+D8183,PLC_Addr-=2;break;//算出H的地址，的Z1
		case  0xA282: u32data.data1[0]=temp2+D8184,PLC_Addr++,u32data.data1[1]=cos_value()+D8185,PLC_Addr-=2;break;//算出H的地址，的Z2
		case  0xA283: u32data.data1[0]=temp2+D8186,PLC_Addr++,u32data.data1[1]=cos_value()+D8187,PLC_Addr-=2;break;//算出H的地址，的Z3
		case  0xA284: u32data.data1[0]=temp2+D8188,PLC_Addr++,u32data.data1[1]=cos_value()+D8189,PLC_Addr-=2;break;//算出H的地址，的Z4
		case  0xA285: u32data.data1[0]=temp2+D8190,PLC_Addr++,u32data.data1[1]=cos_value()+D8191,PLC_Addr-=2;break;//算出H的地址，的Z5
		case  0xA286: u32data.data1[0]=temp2+D8192,PLC_Addr++,u32data.data1[1]=cos_value()+D8193,PLC_Addr-=2;break;//算出H的地址，的Z6
		case  0xA287: u32data.data1[0]=temp2+D8194,PLC_Addr++,u32data.data1[1]=cos_value()+D8195,PLC_Addr-=2;break;//算出H的地址，的Z7 				
		/************************************************K1M0寄存器"Z"*******************************************************************/
		case  0xA482: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0的V0-V3	
		case  0xA483: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K2M0寄存器"Z"*******************************************************************/
		case  0xA484: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0的V0-V3	
		case  0xA485: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K3M0寄存器"Z"*******************************************************************/
		case  0xA486: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0的V0-V3	
		case  0xA487: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K4M0寄存器"Z"*******************************************************************/
		case  0xA488: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0的V0-V3
		case  0xA489: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0的V4-V7		
		/************************************************K5M0寄存器"Z"*******************************************************************/
		case  0xA48A: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000FFFFF,PLC_Addr++;break;//K1M0的V0-V3	
		case  0xA48B: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000FFFFF,PLC_Addr++;break;//K1M0的V4-V7	
		/************************************************K6M0寄存器"Z"*******************************************************************/
		case  0xA48C: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00FFFFFF,PLC_Addr++;break;//K2M0的V0-V3	
		case  0xA48D: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00FFFFFF,PLC_Addr++;break;//K2M0的V4-V7		
		/************************************************K7M0寄存器"Z"*******************************************************************/
		case  0xA48E: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0FFFFFFF,PLC_Addr++;break;//K3M0的V0-V3	
		case  0xA48F: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0FFFFFFF,PLC_Addr++;break;//K3M0的V4-V7		
		/************************************************K8M0寄存器"Z"*******************************************************************/
		case  0xA490: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V0-V3
		case  0xA491: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0的V4-V7	
		/************************************************T寄存器"Z"*******************************************************************/
		case  0xA682: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_T_ADDR+temp),PLC_Addr++;}break;//算出T的地址，的Z0-Z3
		case  0xA683: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_T_ADDR+temp),PLC_Addr++;}break;//算出T的地址，的Z4-Z7
		/************************************************C寄存器"Z"*******************************************************************/
		case  0xA684: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_C_ADDR+temp),PLC_Addr++;}break;//算出C的地址，的Z0-Z3
		case  0xA685: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_C_ADDR+temp),PLC_Addr++;}break;//算出C的地址，的Z4-Z7
		/************************************************D寄存器"Z"*******************************************************************/
		case  0xA686: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=15998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D_ADDR+temp);PLC_Addr++;}break;//算出D的地址，的Z0-Z3
		case  0xA687: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=15998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D_ADDR+temp);PLC_Addr++;}break;//算出D的地址，的Z4-Z7
		case  0xA688: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=13998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D1000_ADDR+temp),PLC_Addr++;}break;//大于等于D1000的Z0-Z3
		case  0xA689: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=13998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D1000_ADDR+temp),PLC_Addr++;}break;//大于等于D1000的Z4-Z7			
	}
	  PLC_Addr+=2;
			return D_data;
}
//=======================================================================================================
// 函数名称: static float float_value(void)
// 功能描述： 
// 输　入:  void      
// 输　出:  输出float数据
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月19日
// 备  注:  
//-------------------------------------------------------------------------------------------------------
// 修改人:
// 日　期:
// 备  注: 
//-------------------------------------------------------------------------------------------------------
//=======================================================================================================
static float float_value(void)                                     
{  
	 u16 Type_F,temp1,temp2,Data1,Data2,Data3,Data4;
	 Data1=*PLC_Addr;
	 PLC_Addr++;
	 Data2=*PLC_Addr;
	 PLC_Addr++;
	 Data3=*PLC_Addr;
	 PLC_Addr++;
	 Data4=*PLC_Addr;
	
	 Type_F   = (Data1 & 0xff00);
	 Type_F  |= (Data2 >> 8);
/************************************/	
	 temp1  = (Data2 << 8);
	 temp1 |=(u8)Data1;
/************************************/
	 temp2  = (Data4 << 8);
	 temp2 |=(u8)Data3;
/************************************/	
	 if(Type_F == 0x8080)      u32data.data1[0]=temp1,u32data.data1[1]=temp2, FLOAT.DATA=(float)u32data.data, PLC_Addr++;//算出K值
	 else if(Type_F == 0x8280) u32data.data1[0]=temp1,u32data.data1[1]=temp2, FLOAT.DATA=(float)u32data.data, PLC_Addr++;//算出H值            
	 else if(Type_F == 0x8680) FLOAT.DATA=PLC_RAMfolta(RAM_D8000_ADDR+temp1),PLC_Addr++;             //算出D的地址 D8000
	 else if(Type_F == 0x8682) FLOAT.DATA=PLC_RAMfolta(RAM_T_ADDR+temp1),PLC_Addr++;                 //算出T的地址 
	 else if(Type_F == 0x8684) FLOAT.DATA=PLC_RAMfolta(RAM_C_ADDR+temp1),PLC_Addr++;                 //算出C的地址
	 else if(Type_F == 0x8686) FLOAT.DATA=PLC_RAMfolta(RAM_D_ADDR+temp1),PLC_Addr++;                 //算出D的地址 
     else if(Type_F == 0x8688) FLOAT.DATA=PLC_RAMfolta(RAM_D1000_ADDR+temp1),PLC_Addr++;             //大于等于D1000
	 return FLOAT.DATA;
}

static void RST_D(void)                                 
{  
	u8 temp,addr,l_value;
	if(PLC_ACC_BIT&0x01)
	{   
		l_value=*PLC_Addr;PLC_Addr++;addr=*PLC_Addr/0x100;
		if(addr==0x86)
		temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0700+temp/2,PLC_16BIT[temp]=0;
		else if(addr==0x88)
		temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x1000+temp/2,PLC_16BIT[temp]=0;
		else if(addr==0x80)
		temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x1000+temp/2+1000,PLC_16BIT[temp]=0;
		else {PLC_PROG_ERROR(M8065,6501);}
	}
	else PLC_Addr+=2;
}

//=======================================================================================================
// 函数名称:  static void target(void)
// 功能描述： 加减法“与”“或”“异或”共用赋值函数	如DEMOV、DEADD、DESUB等指令的结果传递出去
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月23日
// 备  注:  
//=======================================================================================================
static void float_target(void)		
{ 
	u16 temp;
	temp=addr_value() ;            
	PLC_16BIT[temp]=FLOAT.DATA1[0];
	PLC_16BIT[temp+1]=FLOAT.DATA1[1];
	PLC_Addr+=2;
}

static void PID(void)
{
	s16 PVn;  // 测量值
	s16 SV;   // 设定目标Desired value
	s16 Ts;   // 取样时间
	s32 Su;
	s16 KP; // P 
	s16 Ti; // I
	s16 KD; // D
	s16 TD; // 微分增益
	u32 Addr,Addr1; // 地址记录
	u32 csp;        // PID内部计算地址
	if((PLC_ACC_BIT&0X01)==0X01)        //母线成立不
	{
		SV=PLC_RAM16(addr_value_prog());   // 设定值
		PVn=PLC_RAM16(addr_value_prog());  // 测量数据
		Addr=addr_value_prog();            // 读取参数起始地址
		Addr1= addr_value_prog();
		Ts=PLC_RAM16(Addr);                // 取样时间
		KP=PLC_RAM16(Addr+6);  // P
		Ti=PLC_RAM16(Addr+8);  // I
		KD=PLC_RAM16(Addr+10); // D
		TD=PLC_RAM16(Addr+12); // 微分增益
		csp=Addr+14;           // 内部计算起始地址
		 
		PLC_RAM16(csp+14)=KP*((PLC_RAM16(csp)-PLC_RAM16(csp+2))+((Ts/Ti)*PLC_RAM16(csp))+PLC_RAM16(csp+10)); 
		
		PLC_RAM16(csp) = PLC_RAM16(csp+4) - SV;         // 计算本次偏差值    
		
		PLC_RAM16(csp+4)= PLC_RAM16(Addr+4)*PLC_RAM16(csp+6)+(1-PLC_RAM16(Addr+4))*PVn; 
		
		PLC_RAM16(csp+10)=(TD/(Ts+KD*TD))*(-2*PLC_RAM16(csp+6)+PLC_RAM16(csp+4)+PLC_RAM16(csp+8))+((KD*TD)/(Ts+KD*TD))*PLC_RAM16(csp+12);
		
		
		Su=PLC_RAM16(Addr1)+PLC_RAM16(csp+14);
		if(Su>32766)       PLC_RAM16(Addr1)=32767;
		else if(Su<-32767) PLC_RAM16(Addr1)=-32768;
		else PLC_RAM16(Addr1)= Su;
		PLC_RAM16(csp+12)=PLC_RAM16(csp+10);
		PLC_RAM16(csp+8)=PLC_RAM16(csp+6);
		PLC_RAM16(csp+6)=PLC_RAM16(csp+4);
		PLC_RAM16(csp+2)=PLC_RAM16(csp);
	}
}

static void MOV(void)	          //MOV
{
	if(PLC_ACC_BIT&0X01)         //判断条件满足否
	trade=cos_value(),target();  
	else
	PLC_Addr+=4;		              //条件不满足执行跳过程序，减小CPU开销
}

static void DMOV(void)	        //单精度数据传递
{  	
   if(PLC_ACC_BIT&0X01)
   trade=cos_u32_value(),D_target(); 
   else	 
   PLC_Addr+=8;		              //条件不满足执行跳过程序，减小CPU开销
}

static void DEMOV(void)	        //单精度数据传递
{  	
   if(PLC_ACC_BIT&0X01)
   trade=float_value(),float_target(); 
   else	 
   PLC_Addr+=8;		              //条件不满足执行跳过程序，减小CPU开销
}

//======================================================================================================
// 函数名称: static void ZRST(void) 
// 功能描述： ZRST指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void ZRST(void)  
{ 
	u16 temp,temp1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=addr_value();
		temp1=addr_value();
		if(Flag_bit==0x00) 
		{
			for(;temp<=temp1;temp++)
			PLC_BIT_OFF(temp);
		} 
		else
		{		 
			for(;temp<=temp1;temp++)
			PLC_16BIT[temp]=0;
		}
	}
	else PLC_Addr+=4;
}

static void MTR(void)
{
	u16 X,Y,M_Y,K_H,temp=0;
	u8 i,t;	 
	if(PLC_ACC_BIT&0X01)
	{
		X=addr_value();
		Y=addr_value();
		M_Y=addr_value();
		K_H=cos_value();
		for(i=0;i<K_H;i++)
		{
			temp=i*7;
			PLC_BIT_ON(Y+i);
			for(t=0;t<=7;t++)
			(PLC_BIT_TEST(X+t)) ? PLC_BIT_ON(M_Y+temp+t) : PLC_BIT_OFF(M_Y+temp+t);
		}
	}
	else PLC_Addr+=8;
}


static void REFF(void)
{
	 if(PLC_ACC_BIT&0X01)
	 X_DIY=cos_value();
	 else X_DIY=10,PLC_Addr+=2;
}
//======================================================================================================
// 函数名称:  static void DSQR(void)
// 功能描述： 32位开方计算 DSQR
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DSQR(void)
{
   if((PLC_ACC_BIT&0X01)==0X01)
   {
  		trade=(u32)sqrt((double)cos_u32_value());
	    target();	
   } 
}

static void HSCS(void)		//高速计数置位
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp2==temp1)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

//======================================================================================================
// 函数名称:  static void SQR(void)
// 功能描述： 16位右移位 RCR指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void SQR(void)
{
   if((PLC_ACC_BIT&0X01)==0X01)
   {
  		trade=(u16)sqrt((double)cos_value());
	    target();	
   } 
}
//======================================================================================================
// 函数名称:  static void DRCR(void)
// 功能描述： 32位右移位 RCR指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DRCR(void)	                 
{  	
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value(); 
		temp2=cos_u32_value(); 
		trade=temp1>>temp2;
		PLC_Addr-=8;
		D_target();	
		PLC_Addr+=4;
	}
}
//======================================================================================================
// 函数名称:  static void RCR(void)
// 功能描述： 16位右移位 RCR指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void RCR(void)	                 
{  	
	u16 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value(); 
		temp2=cos_value(); 
		trade=temp1>>temp2;
		PLC_Addr-=4;
		target();	
		PLC_Addr+=2;
	}
	else PLC_Addr+=4;                       //没有动作跳过4步程序
}
//======================================================================================================
// 函数名称:  static void DROL(void)
// 功能描述： 32位左移位 RCL指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DRCL(void)	                 
{  	
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value(); 
		temp2=cos_u32_value();  
		trade=temp1<<temp2;
		PLC_Addr-=8;       
		D_target();	
		PLC_Addr+=4;
	}
}
//======================================================================================================
// 函数名称:  static void ROL(void)
// 功能描述： 16位左移位 RCL指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void RCL(void)	                 
{  	
	u16 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value(); 
		temp2=cos_value(); 
		trade=temp1<<temp2;
		PLC_Addr-=4;
		target();	
		PLC_Addr+=2;
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}
//======================================================================================================
// 函数名称:  static void DROR(void)
// 功能描述： 32位循环右位 ROR指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DROR(void)	                 
{  	
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value(); 
		temp2=cos_u32_value(); 
		trade=ROTATE_RIGHT(temp1,32,temp2);
		PLC_Addr-=8;
		D_target();	
		PLC_Addr+=4;
	}
}
//======================================================================================================
// 函数名称:  static void ROR(void)
// 功能描述： 16位循环右位 ROR指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void ROR(void)	                 
{  	
	u16 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value(); 
		temp2=cos_value(); 
		trade=ROTATE_RIGHT(temp1,16,temp2);
		PLC_Addr-=4;
		target();	
		PLC_Addr+=2;
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}

//======================================================================================================
// 函数名称:  static void DROL(void)
// 功能描述： 32位循环左移位 ROL指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DROL(void)	                 
{  	
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value(); 
		temp2=cos_u32_value();  
		trade=ROTATE_LEFT(temp1,32,temp2);
		PLC_Addr-=8;       
		D_target();	
		PLC_Addr+=4;
	}
}
//======================================================================================================
// 函数名称:  static void ROL(void)
// 功能描述： 16位循环左移位 ROL指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void ROL(void)	                 
{  	
	u16 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value(); 
		temp2=cos_value(); 
		trade=ROTATE_LEFT(temp1,16,temp2);
		PLC_Addr-=4;
		target();	
		PLC_Addr+=2;
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}
//======================================================================================================
// 函数名称:  static void DSWAP(void)	
// 功能描述： 32位上下交换 DSWAP指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DSWAP(void)	                 
{  	
	u32 temp;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=cos_u32_value(); 
		trade=swap_u32(temp);
		PLC_Addr-=4;
		D_target();	
	}
}
//=======================================================================================================
// 函数名称:  static void DGBIN(void)	
// 功能描述： 16位上下交换 DGBIN指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年8月4日
// 备  注:  
//=======================================================================================================
static void DGBIN(void)	                 
{  	
	signed int temp;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=cos_value();		  
		trade=GtoB(temp);
		D_target();		 
	}
	else PLC_Addr+=8;                      //没有动作跳过8步程序
}

//=======================================================================================================
// 函数名称:  static void GBIN(void)	
// 功能描述： 16位上下交换 GBIN指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void GBIN(void)	                 
{  	
	signed short int temp;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=cos_value();		  
		trade=(u16)GtoB((unsigned int)temp);
		target();		 
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}
//=======================================================================================================
// 函数名称:  static void DGRY(void)	
// 功能描述： 16位上下交换 DGRY指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年8月4日
// 备  注:  
//=======================================================================================================
static void DGRY(void)	                 
{  	
	signed int temp;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=cos_value();		  
		trade=BtoG(temp);
		D_target();		 
	}
	else PLC_Addr+=8;                      //没有动作跳过8步程序
}

//=======================================================================================================
// 函数名称:  static void GRY(void)	
// 功能描述： 16位上下交换 GRY指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void GRY(void)	                 
{  	
	signed short int temp;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=cos_value();		  
		trade=(u16)BtoG((unsigned int)temp);
		target();		 
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}

//=======================================================================================================
// 函数名称:  static void SWAP(void)	
// 功能描述： 16位上下交换 SWAP指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void SWAP(void)	                 
{  	
	signed short int temp;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp=cos_value();	  
		trade=swap_u16(temp);
		PLC_Addr-=2;
		target();		 
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}

//=======================================================================================================
// 函数名称:  static void SFTR(void)	 
// 功能描述： SFTR指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年8月9日
// 备  注:  
//=======================================================================================================
//static void SFTR(void)	                 
//{  	
//signed short int addr1,addr2,temp1,temp2,temp5,temp6,i;
//	u8 temp3,temp4;
//   if(PLC_ACC_BIT&0X01)
//   {
//		  addr1=addr_value();
//		  Flag_bit=0xff;
//		 
//		  addr2=addr_value();
//		  Flag_bit=0xff;
//		 
//		  temp1=cos_value();
//		  temp5=cos_value();
//		  temp2=temp5+temp4;
//		  temp6=temp1-temp5;
//  		for(i=0;i<temp6;i++)
//		  { 
//				(PLC_BIT_TEST(addr2+i)) ? PLC_BIT_ON(addr2+i) : PLC_BIT_OFF(addr2+i);
//			}
//			for(;i<temp1;i++)
//		  { 
//				(PLC_BIT_TEST(addr1+i)) ? PLC_BIT_ON(addr2+i) : PLC_BIT_OFF(addr2+i);
//			}
//	 }
//	 else PLC_Addr+=8;                      //没有动作跳过4步程序
//}



//=======================================================================================================
// 函数名称:  static void XCH(void)	 
// 功能描述： 16位交换传送 XCH指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void XCH(void)	                 
{  	
	signed short int temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		PLC_Addr-=4;
		trade=temp2;
		D_target();
		trade=temp1;
		D_target();
	}
	else PLC_Addr+=4;                      //没有动作跳过4步程序
}
//=======================================================================================================
// 函数名称:  static void DFMOV(void)	 
// 功能描述： 32位交换传送 DXCH指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DXCH(void)	                 
{  	
	signed int temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();
		PLC_Addr-=8;
		trade=temp2;
		D_target();
		trade=temp1;
		D_target();
	}
}
//=======================================================================================================
// 函数名称:  static void DFMOV(void)	 
// 功能描述： 32位多点传送 DFMOV指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void DFMOV(void)	               
{  	
	signed short int temp,i;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_u32_value();            //要传递的数据
		D_target();                       //把第一个传递出去
		temp=cos_u32_value();             //            <<<-------------|
		PLC_Addr-=4;                      //PLC_Addr-=4是为了调回到上面 |<<-----|
		for(i=1;i<temp;i++)               //                                    |
		{                                 //                                    |
			if(Transfer_bit==1)Transfer=i*32;//                                  |
			else Transfer=i*4;             //                                    |
			PLC_Addr-=4;D_target();        //PLC_Addr-=4是为了传递出去要的位数	  |
		}                                 //                                    |
		PLC_Addr+=2;Transfer=0;           //PLC_Addr+=2是为了跳过上面调回去的---|
	}
}

//=======================================================================================================
// 函数名称:  static void FMOV(void)	 
// 功能描述： 16位多点传送 FMOV指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void FMOV(void)	             
{  	
	signed short int temp,i;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_value();                //要传递的数据
		target();                         //把第一个传递出去
		temp=cos_value();                 //            <<<-------------|
		PLC_Addr-=2;                      //PLC_Addr-=2是为了调回到上面 |<<-----|
		for(i=1;i<temp;i++)               //                                    |
		{                                 //                                    |
			if(Transfer_bit==1)Transfer=i*16;//                                  |
			else Transfer=i*2;             //                                    |
			PLC_Addr-=2;target();          //PLC_Addr-=2是为了传递出去要的位数	  |
		}                                 //                                    |
		PLC_Addr+=2;Transfer=0;           //PLC_Addr+=2是为了跳过上面调回去的---|
	}
	else PLC_Addr+=6;                    //没有动作跳过6步程序
}

//=======================================================================================================
// 函数名称:  static void FMOV(void)	 
// 功能描述： 16位成批传送 BMOV指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年6月27日
// 备  注:  
//=======================================================================================================
static void BMOV(void)	                 
{  	
	signed short int temp,i;
	if((PLC_ACC_BIT&0X01)==0X01)
		{
		trade=cos_value();                //要传递的数据
		target();                         //把第一个传递出去
		temp=cos_value();
		PLC_Addr-=2;                        //<<<---------------------------------|
		for(i=1;i<temp;i++)                 //                                    |
		{                                   //                                    |
			if(Transfer_bit1==1)Transfer=i*16;//                                    |
			else Transfer=i*2;               //                                    |
			PLC_Addr-=4;                     //                 
			trade=cos_value();               //要传递的数据
			if(Transfer_bit==1)Transfer=i*16;
			else Transfer=i*2;
			target();                   //把第传递出去
		}                                   //                                      |
		PLC_Addr+=2;Transfer=0;             //PLC_Addr+=2是为了跳过上面调回去的-----|
	}
	else PLC_Addr+=6;                      //按键没按下就跳过6步程序
}

static void DCML(void)	                 //32位数据取反
{  	
	s32 temp1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		trade=~temp1;
		D_target();
	}
	else PLC_Addr+=8;                     //跳过8步程序
}

static void CML(void)	                    //数据取反
{  	
	signed short int temp1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		trade=~temp1;
		target();
	}
	else PLC_Addr+=4;                   //跳过4步程序
}


u16 bcd[4]={0x1,0x10,0x100,0x1000};
static void SMOV(void)	          //16位比较传送指令
{ 
	u16 temp1,temp2,temp3,temp4,temp5,temp6;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		temp3=cos_value();
		temp4=addr_value();
		temp5=cos_value();
		temp1%=bcd[temp2];
		for(temp6=0;temp6<temp3;temp6++)
		{ 
			temp2--;temp5--;
			PLC_16BIT[temp4]|=(temp1/bcd[temp2])*bcd[temp5];
			if((temp2==1)&&(temp5==1))PLC_16BIT[temp4]|=temp1%0x10*bcd[temp5], temp6=temp3+1;
			else temp1%=bcd[temp2];
		}
	}
	else PLC_Addr+=10;              //跳过10步程序
}

//======================================================================================================
// 函数名称: static void TZCP(void)
// 功能描述：TZCP指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void TZCP(void)
{
	u16 h,min,s,temp,temp1,temp3,temp4,h1,min1,s1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{	
		temp3=addr_value();               //S1 区域比较下限值
		h=PLC_16BIT[temp3];
		min=PLC_16BIT[temp3+1];
		s=PLC_16BIT[temp3+2];
		
		temp4=addr_value();               //S2 区域比较上限值
		h1=PLC_16BIT[temp4];
		min1=PLC_16BIT[temp4+1];
		s1=PLC_16BIT[temp4+2];
		
		temp=addr_value();                //S3 时间比较值
		
		temp1=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp1);PLC_BIT_OFF(temp1+1);PLC_BIT_OFF(temp1+2);  
		if((h>=PLC_16BIT[temp])&&(min>=PLC_16BIT[temp+1])&&(s>PLC_16BIT[temp+2]))
		{PLC_BIT_ON(temp1);}
		else if(((h<=PLC_16BIT[temp])&&(min<=PLC_16BIT[temp+1])&&(s<=PLC_16BIT[temp+1]))&&((h1>=PLC_16BIT[temp])&&(min1>=PLC_16BIT[temp+1])&&(s1>=PLC_16BIT[temp+2])))
		{PLC_BIT_ON(temp1+1);}
		else if((h1<=PLC_16BIT[temp])&&(min1<=PLC_16BIT[temp+1])&&(s1<PLC_16BIT[temp+2]))
		{PLC_BIT_ON(temp1+2);}
	}
}

static void EZCP(void)	          //16位比较传送指令
{ 
	float temp1,temp2,temp3;u32 temp4;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=float_value();
		temp2=float_value();
		temp3=float_value();
		temp4=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp4);PLC_BIT_OFF(temp4+1);PLC_BIT_OFF(temp4+2);
		if(temp1>temp3)       PLC_BIT_ON(temp4+0); 
		else if((temp1<=temp3)&&(temp3<=temp2)) PLC_BIT_ON(temp4+1);
		else if(temp2<temp3)  PLC_BIT_ON(temp4+2); 
	}
	else PLC_Addr+=16;              //跳过16步程序
}

static void DZCP(void)	          //16位比较传送指令
{ 
	s32 temp1,temp2,temp3,temp4;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();
		temp3=cos_u32_value();
		temp4=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp4);PLC_BIT_OFF(temp4+1);PLC_BIT_OFF(temp4+2);
		if(temp1>temp3)       PLC_BIT_ON(temp4); 
		else if((temp1<=temp3)&&(temp3<=temp2)) PLC_BIT_ON(temp4+1);
		else if(temp2<temp3)  PLC_BIT_ON(temp4+2); 
	}
	else PLC_Addr+=16;              //跳过16步程序
}

static void ZCP(void)	            //16位比较传送指令
{ 
	signed short int temp1,temp2,temp3,temp4;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		temp3=cos_value();
		temp4=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp4);PLC_BIT_OFF(temp4+1);PLC_BIT_OFF(temp4+2);
		if(temp1>temp3)       PLC_BIT_ON(temp4); 
		else if((temp1<=temp3)&&(temp3<=temp2)) PLC_BIT_ON(temp4+1);
		else if(temp2<temp3)  PLC_BIT_ON(temp4+2); 
	}
	else PLC_Addr+=8;              //跳过8步程序
}
//======================================================================================================
// 函数名称: static void TCMP(void)
// 功能描述：TCMP指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void TCMP(void)
{   
	u16 h,min,s,temp,temp1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{	
		h=cos_value();min=cos_value();s=cos_value();
		temp=addr_value();
		temp1=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp1);PLC_BIT_OFF(temp1+1);PLC_BIT_OFF(temp1+2);  
		if((h>=PLC_16BIT[temp])&&(min>=PLC_16BIT[temp+1])&&(s>PLC_16BIT[temp+2]))
		{PLC_BIT_ON(temp1);}
		else if((h==PLC_16BIT[temp])&&(min==PLC_16BIT[temp+1])&&(s==PLC_16BIT[temp+2]))
		{PLC_BIT_ON(temp1+1);}
		else if((h<=PLC_16BIT[temp])&&(min<=PLC_16BIT[temp+1])&&(s<PLC_16BIT[temp+2]))
		{PLC_BIT_ON(temp1+2);}
	}
	else PLC_Addr+=10;
}

//======================================================================================================
// 函数名称: static void ECMP(void)
// 功能描述：ECMP指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void ECMP(void)	          //浮点比较传送指令
{ 
	signed short int temp3;
	static float temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{	
		temp1=float_value();
		temp2=float_value();
		temp3=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp3);PLC_BIT_OFF(temp3+1);PLC_BIT_OFF(temp3+2);
		if(temp1>temp2)       PLC_BIT_ON(temp3); 
		else if(temp1==temp2) PLC_BIT_ON(temp3+1);
		else if(temp1<temp2)  PLC_BIT_ON(temp3+2); 
	}
	else PLC_Addr+=12;              //跳过12步程序
}

//======================================================================================================
// 函数名称: static void DCMP(void)
// 功能描述：DCMP指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void DCMP(void)	          //32位比较传送指令
{ 
	signed short int temp3;
	static int temp1,temp2;
	if(PLC_ACC_BIT&0X01)
	{	
		temp1=cos_u32_value();
		temp2=cos_u32_value();
		temp3=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp3);PLC_BIT_OFF(temp3+1);PLC_BIT_OFF(temp3+2);
		if(temp1>temp2)       PLC_BIT_ON(temp3+0); 
		else if(temp1==temp2) PLC_BIT_ON(temp3+1);
		else if(temp1<temp2)  PLC_BIT_ON(temp3+2); 
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void DCMPP(void)	  
{ 
	signed short int temp3;
	static int temp1,temp2;
	if(PLC_LDP_TEST())        //上升沿判断
	{ 
		temp1=cos_u32_value();
		temp2=cos_u32_value();
		temp3=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp3);PLC_BIT_OFF(temp3+1);PLC_BIT_OFF(temp3+2);
		if(temp1>temp2)       PLC_BIT_ON(temp3+0); 
		else if(temp1==temp2) PLC_BIT_ON(temp3+1);
		else if(temp1<temp2)  PLC_BIT_ON(temp3+2);  
	}
	else
	PLC_Addr+=12;		         //条件不满足执行跳过程序，减小CPU开销
}
//======================================================================================================
// 函数名称: static void CMP(void)
// 功能描述：CMP指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void CMP(void)	          //16位比较传送指令
{ 
	signed short int temp1,temp2,temp3;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		temp3=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp3);PLC_BIT_OFF(temp3+1);PLC_BIT_OFF(temp3+2);
		if(temp1>temp2)       PLC_BIT_ON(temp3); 
		else if(temp1==temp2) PLC_BIT_ON(temp3+1);
		else if(temp1<temp2)  PLC_BIT_ON(temp3+2); 	 
	}
	else PLC_Addr+=6;              //跳过6步程序
}
//======================================================================================================
// 函数名称: static void CMP_P(void)
// 功能描述：CMPP指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年8月26日
// 备  注:  
//=======================================================================================================
static void CMPP(void)	  
{ 
	signed short int temp1,temp2,temp3;
	if(PLC_LDP_TEST())                       //上升沿判断
	{ 
		temp1=cos_value();
		temp2=cos_value();
		temp3=addr_value();Flag_bit=0XFF;
		PLC_BIT_OFF(temp3);PLC_BIT_OFF(temp3+1);PLC_BIT_OFF(temp3+2);
		if(temp1>temp2)       PLC_BIT_ON(temp3); 
		else if(temp1==temp2) PLC_BIT_ON(temp3+1);
		else if(temp1<temp2)  PLC_BIT_ON(temp3+2); 	 
	}
	else
	PLC_Addr+=6;		                          //条件不满足执行跳过程序，减小CPU开销
}


static void DINC(void)	         //32位逻辑运算 加1指令
{ 
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=(u32)cos_u32_value()+1;
		PLC_Addr-=4;
		D_target();
	}
	else PLC_Addr+=4;              //跳过4步程序
}

static void DINC_P(void)	  //CALLP
{ 
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)==off)//上升沿判断
	{ 
		if(PLC_ACC_BIT&0X01)			                      //当前值判断
		{
			PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);
			trade=(u32)cos_u32_value()+1;
			PLC_Addr-=4;
			trade++;
			D_target();
		}
		else PLC_Addr+=4;
	}
	else
	{
		if(!((PLC_ACC_BIT&0x01)==0x01))						 //当前值判断
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);//
		PLC_Addr+=4;		                           //条件不满足执行跳过程序，减小CPU开销
	} 
}

static void INC(void)	            //逻辑运算 加1指令
{ 
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_value();
		PLC_Addr-=2;
		trade++;
		target();
	}
	else PLC_Addr+=2;              //跳过2步程序
}

static void INCP(void)	  //INCP
{ 
	if(PLC_LDP_TEST())//上升沿判断
	{ 
		trade=cos_value();
		PLC_Addr-=2;
		trade++;
		target();
	}
	else
	{
		PLC_Addr+=2;		                            //条件不满足执行跳过程序，减小CPU开销
	} 
}

static void DDEC(void)                             //32位逻辑运算 减1指令
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_u32_value()-1;
		PLC_Addr-=4;
		D_target();
	}
	else PLC_Addr+=4;              //跳过4步程序
}

static void DEC(void)             //逻辑运算 减1指令
{
	if(PLC_ACC_BIT&0X01)
	{
		trade=cos_value()-1;
		PLC_Addr-=2;
		target();
	}
	else PLC_Addr+=2;              //跳过2步程序
}

static void DECP(void)	          //INCP
{ 
	if(PLC_LDP_TEST())              //上升沿判断
	{ 
		trade=cos_value()-1;
		PLC_Addr-=2;
		target();
	}
	else
	PLC_Addr+=2;		                           //条件不满足执行跳过程序，减小CPU开销
}

static void DNEG(void)                             //32位逻辑运算取补码
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=0-cos_u32_value();
		PLC_Addr-=4;
		D_target();		 
	}
	else PLC_Addr+=4;              //跳过4步程序
}

static void NEG(void)                             //逻辑运算取补码
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=0-cos_value();
		PLC_Addr-=2;
		target();
	}
	else PLC_Addr+=2;              //跳过2步程序
}

static void DWAND(void)	                          //逻辑运算“与”
{ 
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();;
		trade=temp1&temp2;
		D_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void WAND(void)	                          //逻辑运算“与”
{ 
	signed short int temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		trade=temp1&temp2;
		target();
	}
	else PLC_Addr+=6;              //跳过6步程序
}

static void DWOR(void)	                           //逻辑运算“或”
{  
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();;
		trade=temp1|temp2;
		D_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void WOR(void)	                           //逻辑运算“或”
{  
	signed short int temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		trade=temp1|temp2;
		target();
	}
	else PLC_Addr+=6;              //跳过6步程序
}

static void DWXOR(void)	                          //逻辑运算“异或”
{ 
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();
		trade=temp1^temp2;
		D_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void WXOR(void)	                          //逻辑运算“异或”
{ 
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_value()^cos_value();
		target();
	}
	else PLC_Addr+=6;              //跳过6步程序
}
//======================================================================================================
// 函数名称: static void TADD(void)
// 功能描述：TADD指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void TADD(void)
{  
	u16 temp,temp1,temp2;
	if(PLC_ACC_BIT&0X01)
	{ 
		temp1=addr_value();              
		temp2=addr_value();  
		temp=addr_value();
		PLC_16BIT[temp]=PLC_16BIT[temp1]+PLC_16BIT[temp2];
		PLC_16BIT[temp+1]=PLC_16BIT[temp1+1]+PLC_16BIT[temp2+1];
		PLC_16BIT[temp+2]=PLC_16BIT[temp1+2]+PLC_16BIT[temp2+2];
	}
}
void MEAN(void)
{
	u16 temp,temp2;uint64_t data;u32 temp1;
	if(PLC_ACC_BIT&0X01)
	{ 
		temp1=addr_value_prog();             
		PLC_Addr+=2;        //些跳过输出  
		temp2=cos_value();
		if(Flag_bit==0xff)  //是不是K4M0之类的寄存器
		{
			for(temp=0;temp<temp2;temp++)
			{data+=PLC_RAM16(temp1+temp*2);}
			PLC_Addr-=4;        //跳回输出
		}
		else
		{  
			data=(u16)temp1;
			PLC_Addr-=4;   //回跳
			for(temp=1;temp<temp2;temp++)
			{ 
				PLC_Addr-=2;
				Transfer=temp*16;
				data+=addr_value_prog();
			}
			Flag_bit=0xff;
		}
		trade=data/temp2;
		target();
		PLC_Addr+=2;        
	}
	else PLC_Addr+=6;
}
//=======================================================================================================
// 函数名称:  static void ADD(void)	 
// 功能描述： 16位交换传送 ADD指令  加法
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年8月4日
// 备  注:  
//=======================================================================================================
static void ADD(void)	   
{ 
	if(PLC_ACC_BIT&0X01)
	{
		trade=cos_value()+cos_value();
		target();
	}
	else PLC_Addr+=6;              //跳过6步程序
}
//=======================================================================================================
// 函数名称:  static void ALT(void)	 
// 功能描述： 16位交换传送 ALT指令
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年8月4日
// 备  注:  
//=======================================================================================================
static void ALT(void)	                 
{  	
	signed int temp;
	if(PLC_ACC_BIT&0X01)
	{
		temp=addr_value(); 
		if(PLC_BIT_TEST(temp))
		PLC_BIT_OFF(temp);
		else
		PLC_BIT_ON(temp);
	}
	else PLC_Addr+=2;                      //没有动作跳过2步程序
}

//======================================================================================================
// 函数名称: static void TRD(void)
// 功能描述：TRD指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void TRD(void)
{
	u16 temp;
	if(PLC_ACC_BIT&0X01)
	{
		temp=addr_value();
		PLC_16BIT[temp]=PLC_16BIT[0x712];
		PLC_16BIT[temp+1]=PLC_16BIT[0x711];
		PLC_16BIT[temp+2]=PLC_16BIT[0x710];
		PLC_16BIT[temp+3]=PLC_16BIT[0x70F];
		PLC_16BIT[temp+4]=PLC_16BIT[0x70E];
		PLC_16BIT[temp+5]=PLC_16BIT[0X70D];
		PLC_16BIT[temp+6]=PLC_16BIT[0X713];
		//读取年月日时分秒和星期
	}
	else PLC_Addr+=2;              //跳过2步程序
}

//======================================================================================================
// 函数名称: static void TWR(void)
// 功能描述：TWR指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void TWR(void)
{
	u16 temp;
	if(PLC_ACC_BIT&0X01)
	{
		temp=addr_value();
		RTC_Set(PLC_16BIT[temp],PLC_16BIT[temp+1],PLC_16BIT[temp+2],PLC_16BIT[temp+3],PLC_16BIT[temp+4],PLC_16BIT[temp+5]);
		//写入年月日时分秒
	}
	else PLC_Addr+=2;              //跳过2步程序
}

//======================================================================================================
// 函数名称: static void TSUB(void)
// 功能描述：TSUB指令函数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2015年7月26日
// 备  注:  
//=======================================================================================================
static void TSUB(void)
{
	u16 temp,temp1,temp2;
	if(PLC_ACC_BIT&0X01)
	{
		temp1=addr_value();  
		temp2=addr_value();  
		temp=addr_value();
		PLC_16BIT[temp]=PLC_16BIT[temp1]-PLC_16BIT[temp2];
		PLC_16BIT[temp+1]=PLC_16BIT[temp1+1]-PLC_16BIT[temp2+1];
		PLC_16BIT[temp+2]=PLC_16BIT[temp1+2]-PLC_16BIT[temp2+2];
		//时分秒减法
	}
}

static void SUB(void)	   //减法
{ 
	if(PLC_ACC_BIT&0X01)
	{
		PLC_Err=PLC_Addr;
		trade=cos_value()-cos_value();
		target();
	} 
	else PLC_Addr+=6;              //跳过6步程序
}

static void DBCD(void)	            //二进制转换DBCD
{ 
	signed int can1,add1,add2,add3,add4,buffer1,buffer2,buffer3,buffer4;
	if(PLC_ACC_BIT&0X01)
	{
		PLC_Err=PLC_Addr;
		can1=cos_u32_value();
		add1=can1%10;
		add2=can1/10;
		add2=add2%10;
		add3=can1/100;
		add3=add3%10;
		add4=can1/1000;
		add4=add4%10;  
		
		buffer1=can1/10000;
		buffer1=buffer1%10;
		
		buffer2=can1/100000;
		buffer2=buffer2%10;
		
		buffer3=can1/1000000;
		buffer3=buffer3%10;
		
		buffer4=can1/10000000;
		buffer4=buffer4%10;
		
		trade=buffer4*16*256*65536+buffer3*256*65536+buffer2*16*65536+buffer1*65536+add4*16*256+add3*256+add2*16+add1;
		D_target();
	}
	else PLC_Addr+=8;              //跳过4步程序
}

static void BCD(void)	            //二进制转换BCD
{ 
	signed short Ia, Ic;
	if((PLC_ACC_BIT&0X01)==0X01)
	{ 
		PLC_Err=PLC_Addr;
		Ic = cos_value();
		Ia   = (Ic / 1000) << 12;
		Ic  %= 1000;
		Ia  |= (Ic / 100 ) << 8;
		Ic  %= 100;
		Ia  |= (Ic / 10 ) << 4;
		Ic  %= 10;
		Ia  |=  Ic;
		trade=Ia;
		target();
	}
	else PLC_Addr+=4;              //跳过4步程序
}

static void DBIN(void)	         //二进制转换DBIN
{ 
	signed int can1,add1,add2,add3,add4,buffer1,buffer2,buffer3,buffer4;
	if(PLC_ACC_BIT&0X01)
	{  
		PLC_Err=PLC_Addr;
		can1=cos_u32_value();
		add1=can1%16;
		add2=can1/16;
		add2=add2%16;
		add3=can1/256;
		add3=add3%16;
		add4=can1/(16*256);
		add4=add4%16;
		
		can1=can1/65536;
		buffer1=can1%16;
		buffer2=can1/16;
		buffer2=buffer2%16;
		buffer3=can1/256;
		buffer3=buffer3%16;
		buffer4=can1/(16*256);
		buffer4=buffer4%16;
		
		trade=buffer4*10000000+buffer3*1000000+buffer2*100000+buffer1*10000+add4*1000+add3*100+add2*10+add1;
		
		D_target();
	}
	else PLC_Addr+=8;              //跳过4步程序
}

static void BIN(void)	            //二进制转换BIN
{ 
	signed short Ia, Ic;
	if((PLC_ACC_BIT&0X01)==0X01)
	{	
		PLC_Err=PLC_Addr;
		Ic = cos_value();
		Ia   = ((Ic >> 12) & 0x0f) * 1000;
		Ia  += ((Ic >> 8 ) & 0x0f) * 100;
		Ia  += ((Ic >> 4 ) & 0x0f) * 10;
		Ia  +=   Ic        & 0x0f;
		trade=Ia;
		target();
	}
	else PLC_Addr+=4;              //跳过4步程序
}
 
static void MUL(void)	 //乘法
{ 
	signed int temp1,temp2;u32 temp3;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		PLC_Err=PLC_Addr;
		temp1=cos_value();
		temp2=cos_value();
		temp3=addr_value_prog(); 
		PLC_RAM32(temp3)=temp1*temp2;
	}
	else PLC_Addr+=6;              //跳过6步程序
}
 

static void DIV(void)	 //除法
{  
	signed short int temp1,temp2,temp3;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		PLC_Err=PLC_Addr;
		temp1=cos_value();
		temp2=cos_value();
		temp3=addr_value();
		PLC_16BIT[temp3]=temp1/temp2;
		PLC_16BIT[temp3+1]=temp1%temp2;
	}
	else PLC_Addr+=6;              //跳过6步程序
}

static void DADD(void)
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		PLC_Err=PLC_Addr;
		trade=cos_u32_value()+cos_u32_value();
		D_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void DSUB(void)
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{  
		PLC_Err=PLC_Addr;
		trade=cos_u32_value()-cos_u32_value();
		D_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void DMUL(void)    
{	 
	signed short int temp;
	if(PLC_ACC_BIT&0X01)
	{
		u64data.data=(int64_t)(cos_u32_value()*cos_u32_value()); //些存放64位共同体
		temp=addr_value(); PLC_Addr+=2;               
		PLC_16BIT[temp] = u64data.data1[0];         //输出
		PLC_16BIT[temp+1] = u64data.data1[1]; 
		PLC_16BIT[temp+2] = u64data.data1[2]; 
		PLC_16BIT[temp+3] = u64data.data1[3];	
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void DDIV(void)
{
	signed short int temp1,temp2,temp3;
	if(PLC_ACC_BIT&0X01)
	{
		temp1 = cos_u32_value();             
		temp2 = cos_u32_value();		
		
		u32data.data=temp1/temp2;                   //商
		u32data1.data=temp1%temp2;                  //余数
		
		temp3=addr_value() ;PLC_Addr+=2;                         
		PLC_16BIT[temp3] = u32data.data1[0];         //输出
		PLC_16BIT[temp3+1]=u32data.data1[1]; 
		PLC_16BIT[temp3+2]=u32data1.data1[0]; 
		PLC_16BIT[temp3+3]=u32data1.data1[1];	
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void DFLT(void)	                 //整数转浮点
{  	
	signed int temp1;
	if(PLC_ACC_BIT&0X01)
	{
		temp1=cos_u32_value();
		trade1=(float)temp1;					
		float_target();
	}
	else PLC_Addr+=8;              //跳过8步程序
}
//======================================================================================================
// 函数名称:  static void DESQR(void)
// 功能描述： 浮点二进制换整数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void DINT(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade=(u32)float_value();
		D_target();
	} 
	else PLC_Addr+=8;              //跳过8步程序
}
//======================================================================================================
// 函数名称:  static void INT(void)
// 功能描述： 浮点二进制换整数
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void INT(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade=(u16)float_value();
		PLC_Addr-=2; 
		target();
	} 
	else PLC_Addr+=4;              //跳过4步程序
}
//======================================================================================================
// 函数名称:  static void FLT(void)
// 功能描述： 整数转浮点 FLT
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void FLT(void)	                
{  	
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)cos_value();			
		float_target();PLC_Addr-=2; 
	}
	else PLC_Addr+=4;              //跳过4步程序
}

//======================================================================================================
// 函数名称:  static void DTAN(void)
// 功能描述： 浮点正切 DTAN
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void DTAN(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)tan((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //跳过8步程序
}
//======================================================================================================
// 函数名称:  static void DCOS(void)
// 功能描述： 浮点正余 DCOS
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void DCOS(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)cos((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //跳过8步程序
}

//======================================================================================================
// 函数名称:  static void DSIN(void)
// 功能描述： 浮点正弦  DSIN
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void DSIN(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)sin((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //跳过8步程序
}
//======================================================================================================
// 函数名称:  static void DESQR(void)
// 功能描述： 浮点开方计算 DESQR
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void DESQR(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)sqrt((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //跳过8步程序
}

//======================================================================================================
// 函数名称:  static void DEADD(void)	
// 功能描述： 浮点加法运算 指令DEADD
// 输　入:  void      
// 输　出:  void     
// 全局变量:  
// 调用模块: 
// 作　者:  小小晟
// 日　期:  2014年6月27日
// 备  注:  
//=======================================================================================================
static void DEADD(void)	 
{  
	if(PLC_ACC_BIT&0X01)
	{
		trade1=float_value()+ float_value(); 						
		float_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}


static void DESUB(void)	           //浮点减法运算 
{  
	float temp1,temp2;
	if(PLC_ACC_BIT&0X01)
	{
		temp1 = float_value();              
		temp2 = float_value(); 
		
		trade1=temp1-temp2;					
		float_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}



static void DEDIV(void)	           //浮点除法运算 
{  
	float temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1 = float_value();              
		temp2 = float_value(); 
		
		trade1=temp1/temp2;					
		float_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

static void DEMUL(void)	                //浮点乘法运算
{  
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade1=float_value()*float_value();					
		float_target();
	}
	else PLC_Addr+=12;              //跳过12步程序
}

u16 DE[16]={0x0001,0x0003,0x0007,0x000F,0X001F,0X003F,0X007F,0X00FF,0X01FF,0X03FF,0X07FF,0X0FFF,0X1FFF,0X3FFF,0X7FFF,0XFFFF};	
void DECO()
{
  u8 i,t=1;
	int temp1=cos_value();      //读取二进制
	int temp2=addr_value();     
	int n=cos_value();          //解码原件长度  
	temp1&=DE[n-1];             //解码
  for(i=0;i<n;i++)            //计算2的n次方
	{t*=2;}		 
//	if()
  PLC_16BIT[temp2]=1<<(temp1-1);	
}


void PLSY(void)
{  
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	static  u8 addr;
	long sys;
	signed short temp,PUS_TOTAL=0;	
	if(PLC_ACC_BIT&0X01)               //判断条件满足否
	{ 
		temp=cos_value();               //脉冲频率
		PUS_TOTAL=cos_value();          //脉冲计数
		addr=*PLC_Addr;                 //输出地址
		PLC_Addr+=2;
		if(PLC_16BIT[0x078C] >= PUS_TOTAL)PLC_BIT_ON(M8029);
		
		if((addr==0x00)&&(!(PLC_BIT_TEST(M8145)))&&(Y0P==0))           //Y0
		{
			PLC_BIT_ON(0x500);           //Y00 ON 
			PLC_BIT_OFF(M8029);          //M8029
// 			PLC_RAM32(0X20005F1C)=0;     //D8140
			Y0P=1;
			Plus_CMP0=PUS_TOTAL; 
			sys=72000000/((71+1)*(temp*2))-1;
			//IO口初始化
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;      //A7 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
			TIM_DeInit(TIM3);
			TIM_TimeBaseStructure.TIM_Period=sys;		 								 // 自动重装载寄存器周期的值(计数值) 
			// 累计 TIM_Period个频率后产生一个更新或者中断 
			TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);				     // 时钟预分频数 72M/72 
			TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		 // 采样分频 
			TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; // 向上计数模式 
			TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
			TIM_ClearFlag(TIM3, TIM_FLAG_Update);							    	 // 清除溢出中断标志 
			TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
			TIM_Cmd(TIM3, ENABLE);																		 // 开启时钟                 
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);		 // 先关闭等待使用			      
			PLC_BIT_ON(M8147);                                        // M8147 
		}	
	}
	else  
	{  
		PLC_Addr+=4;
		addr=*PLC_Addr;          //输出地址
		PLC_Addr+=2;
		if(addr==0x00)                //Y0
		{
			if(Y0P==1)
			{ 
				TIM_Cmd(TIM5, DISABLE); 
				TIM_Cmd(TIM3, DISABLE);
				TIM_ClearFlag(TIM3, TIM_FLAG_Update);
				TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
							
// 				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  //A2
// 				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
// 				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 				GPIO_Init(GPIOC, &GPIO_InitStructure);						
				Y0P=0;
				PLC_BIT_OFF(0x500);    //Y00 OFF
				PLC_BIT_OFF(M8147);    //M8147
			}
		}
		
	}
}

// static void PWM(void)   //ADD 20160619
// {
//    u16 temp1,temp2;
//    if((PLC_ACC_BIT&0X01)==0X01)
//    {
// 		  temp1=2000000/cos_value();
// 	    temp2=2000000/cos_value();
//       TIM3->ARR = temp2-1;
// 			  switch(Y_addr())
// 			  {
// 				 case 0x051c: TIM3->CCR1 = temp1-1;PLC_BIT_ON(0X00c0,28);TIM3->CCER=TIM_OutputState_Enable;	 break;  //PWM Y34
// 				 case 0x051d: TIM3->CCR2 = temp1-1;PLC_BIT_ON(0X00c0,29);TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);  break;  //PWM Y35
// 				 case 0x051e: TIM3->CCR3 = temp1-1;PLC_BIT_ON(0X00c0,30);TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);  break;  //PWM Y36
//          case 0x051f: TIM3->CCR4 = temp1-1;PLC_BIT_ON(0X00c0,31);TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);  break;  //PWM Y37 
// 			  }
// 	 }
// 	 else
// 	 {
// 		  PLC_Addr+=4;		                   //跳过4步程序 条件不满足执行跳过程序，减小CPU开销              
// 			switch(Y_addr())
// 			{
// 				case 0x051c:  PLC_BIT_OFF(0X00c0,28);  break;                     //PWM Y34
// 				case 0x051d:  PLC_BIT_OFF(0X00c0,29);  break;                       //PWM Y35
// 				case 0x051e:  PLC_BIT_OFF(0X00c0,30);  break;                       //PWM Y36
// 				case 0x051f:  PLC_BIT_OFF(0X00c0,31);  break;                       //PWM Y37
// 			}
// 	 }		 
// }


// void PLSY(void)
// {  
// 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//   TIM_OCInitTypeDef  TIM_OCInitStructure;
//   GPIO_InitTypeDef GPIO_InitStructure;
// 	long div_c;
//   signed short temp,PUS_TOTAL=0;	
// 	static  u8 Y0P,Y1P,addr;;
//    if(PLC_ACC_BIT&0X01)               //判断条件满足否
//    { 
// 		  temp=cos_value();               //脉冲频率
// 	    PUS_TOTAL=cos_value();            //脉冲计数
// 		  addr=*PLC_Addr;                 //输出地址
// 		  PLC_Addr+=2;
//       if((addr==0x00)&&(PLC_BIT_TEST(0x00E0,145))&&(Y0P==0))                //Y0
//       {
// 				 PLC_BIT_ON(0XC0,0);                //Y00 ON
// 			   PLC_BIT_OFF(0X00E0,29);            //M8029
//          PLC_RAM32(0X20005E90)=0;           //D8140
//          Y0P=1;
// 				 
// 	       div_c=75497472/temp;
// 	       div_c=div_c/65536;
// 	       temp=75497472/(temp*(div_c+1));
//          temp--;

//        //只初始化一次  Y0 
// 	       TIM_DeInit(TIM5);
//        //TIM5 clock enable   PWM MODE
// 	       RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); 
// 	       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
// 	       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
// 	       GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
// 	       GPIO_Init(GPIOA, &GPIO_InitStructure);  

// 	       TIM_TimeBaseStructure.TIM_Period = div_c;
// 	       TIM_TimeBaseStructure.TIM_Prescaler = temp;
// 	       TIM_TimeBaseStructure.TIM_ClockDivision = 0;
// 	       TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;

// 	       TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

// 	       // PWM1 Mode configuration: Channel3 
// 	       TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
// 	       TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
// 	       TIM_OCInitStructure.TIM_Pulse = temp/2;
// 	       TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

// 	       TIM_OC3Init(TIM5, &TIM_OCInitStructure);

// 	       TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);
// 	       PLC_16BIT[0x732]=PUS_TOTAL; pus_cnt0=0;	 pus0=PUS_TOTAL;
// 	       // TIM3 clock source enable  ETR MODE	  
// 	       if(PUS_TOTAL>0)
// 	       {
// 	          if(PUS_TOTAL>1)
//             {
//                RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//                TIM_TimeBaseStructure.TIM_Period = PUS_TOTAL-1;
//                TIM_TimeBaseStructure.TIM_Prescaler = 0x00;
//                TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
//                TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//                TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  // Time base configuration

//                TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

//                TIM_SetCounter(TIM3, 0);
//                TIM_ClearFlag(TIM3, TIM_FLAG_Update);
//                TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
//                TIM_Cmd(TIM3, ENABLE);
//             }
//             else
//             TIM_ITConfig(TIM5, TIM_IT_CC2, ENABLE);//打开中断，中断需要这行代码
//         }
//      // TIM5 enable counter
//        TIM_Cmd(TIM5, ENABLE); 
//        PLC_BIT_ON(0X00E0,147);    //M8147 
//       }
//       if((addr==0x01)&&(PLC_BIT_TEST(0x00E0,146)))                //Y1
//       {
// 		    PLC_BIT_ON(0XC0,1);                         //Y01 ON
// 				PLC_BIT_OFF(0X00E0,29);                       //M8029
//         PLC_RAM32(0X20005E92)=0;                      //D8140
//         Y1P=1;
// 	      div_c=75497472/temp;
// 	      div_c=div_c/65536;
// 	      temp=75497472/(temp*(div_c+1));
//         temp--;
// 	      TIM_DeInit(TIM8);
//         //PWM MODE
//         RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE); 
//         GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//         GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//         GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//         GPIO_Init(GPIOC, &GPIO_InitStructure);
// 				
//         TIM_TimeBaseStructure.TIM_Prescaler = div_c;
//         TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
//         TIM_TimeBaseStructure.TIM_Period = temp;
//         PLC_16BIT[0x734]=PUS_TOTAL;
//         TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//         TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

//         TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
//         temp=(temp+1)/2;
//         // Channel 1, 2,3 and 4 Configuration in PWM mode 
//         TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
//         TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//         TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
//         TIM_OCInitStructure.TIM_Pulse =temp;
//         TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
//         TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
//         TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
//         TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

//         TIM_OC3Init(TIM8, &TIM_OCInitStructure);
//         //TIM1 Main Output Enable 
//         TIM_CtrlPWMOutputs(TIM8, ENABLE);
//         PLC_16BIT[0x734]=PUS_TOTAL; pus_cnt1=0;pus1=PUS_TOTAL;

//         if(PUS_TOTAL>0)
//         {
//            // TIM4 clock source enable  ETR MODE	  
//            if(PUS_TOTAL>1)
//            {
//               RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//               TIM_TimeBaseStructure.TIM_Period = PUS_TOTAL-1;
//               TIM_TimeBaseStructure.TIM_Prescaler = 0x00;
//               TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
//               TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//               TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);  // Time base configuration

//               TIM_ETRClockMode2Config(TIM4, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

//               TIM_SetCounter(TIM4, 0);
//               TIM_ClearFlag(TIM4, TIM_FLAG_Update);
//               TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

//               TIM_Cmd(TIM4, ENABLE);
//             }
//             else
//             TIM8->DIER  = 0x02;   // 使能 CC1 中断
//          }
//          // TIM1 counter enable 
//           TIM_Cmd(TIM8, ENABLE); 
//           PLC_BIT_ON(0X00E0,148);    //M8148
// 	   }
//    }
//    else  
//    {   
//   		PLC_Addr+=4;
//       addr=*PLC_Addr;          //输出地址
// 		  PLC_Addr+=2;
//       if(addr==0x00)                //Y0
//       {
//           if(Y0P==1)
//           { 
// 				    	 TIM_Cmd(TIM5, DISABLE); 
// 				    	 TIM_Cmd(TIM3, DISABLE);
// 				    	 TIM_ClearFlag(TIM3, TIM_FLAG_Update);
// 				    	 TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
//   
// 				    	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
// 				    	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
// 				    	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 				    	 GPIO_Init(GPIOA, &GPIO_InitStructure);						
// //				    	 rsty(0);
// 				    	 Y0P=0;
// 							 PLC_BIT_OFF(0x00C0,0);      //Y00 OFF
// 				    	 PLC_BIT_OFF(0X00E0,147);    //M8147
//           }
//       }
//       if(addr==0x01)                //Y0
//       {
// 			    if(Y1P==1)
// 				  {
// 				    	 TIM_Cmd(TIM8, DISABLE); 
// 				    	 TIM_Cmd(TIM4, DISABLE);
// 				    	 TIM_ClearFlag(TIM4, TIM_FLAG_Update);
// 				    	 TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
// 	
// 				    	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
// 				    	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
// 				    	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 				    	 GPIO_Init(GPIOC, &GPIO_InitStructure);
//    
// //				    	 rsty(1);
// 				    	 Y1P=0;
// 							 PLC_BIT_OFF(0x00C0,1);      //Y01 OFF
// 				    	 PLC_BIT_OFF(0X00E0,148);    //M8148
//            }
//       }
//    }
// }




static void Damount(void)	 //32位等于比较指令
{  
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)  //为STL状态区
	{ 
		PLC_ACC_BIT<<=1;		
		if((temp1==temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))      
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1==temp2)     //当前值判断
		PLC_ACC_BIT|=1;
	}
}


static void amount(void)	 //16位等于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)     //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1==temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1==temp2) 					//当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void amount_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1==temp2)||(PLC_ACC_BIT&0X01))  //当前值判断
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 

}

static void Damount_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1==temp2)||(PLC_ACC_BIT&0X01))  //当前值判断
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Damount_and(void)	 //32位AND等于比较
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1==temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void amount_and(void)	 //16位AND等于比较
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1==temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dbig(void)		    //32位大于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)                       //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))    
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>temp2) 						               //当前值判断  陈凡修改"<"改成了">"
		PLC_ACC_BIT|=1;
	}
}

static void big(void)		     //16位大于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)                       //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))      
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>temp2) 						               //当前值判断 陈凡修改"<"改成了">"
		PLC_ACC_BIT|=1;
	}
}

static void big_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1>temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dbig_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1>=temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dbig_and(void)		//32位AND大于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1>temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void big_and(void)		//16位AND大于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1>temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dless(void)	     //32位小于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)                       //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))      
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<temp2) 						               //当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void less(void)	     //小于比较
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)                       //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))    
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<temp2) 						               //当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void less_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1<temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dless_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1<temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dless_and(void)	   //32位AND小于比较
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1<temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void less_and(void)	   //16位AND小于比较
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1<temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dless_amount(void)	     //32位小于等于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)            //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<=temp2) 						    //当前值判断
		PLC_ACC_BIT|=1;
	}
}	

static void less_amount(void)	      //16位小于等于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)          //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<=temp2) 						               //当前值判断
		PLC_ACC_BIT|=1;
	}
}	

static void less_amount_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1<=temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dless_amount_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1<=temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dless_amount_and(void)	   //32位AND小于等于比较
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1<=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void less_amount_and(void)	     //16位AND小于等于比较
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1<=temp2)&&(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE;
}

static void Dbig_amount(void)	     //32位大于等于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)            //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>=temp2) 						               //当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void big_amount(void)	     //16位大于等于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)            //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>=temp2) 						               //当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void big_amount_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1>=temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dbig_amount_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1>=temp2)||(PLC_ACC_BIT&0X01))    
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dbig_amount_and(void)	   //32位AND大于等于比较
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1>=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void big_amount_and(void)	   //16位AND大于等于比较
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1>=temp2)&&(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dno_amount(void)	   //32位不等于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)            //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1!=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1!=temp2) 					 //当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void no_amount(void)	    //16位不等于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)            //为STL状态区
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1!=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1!=temp2) 						               //当前值判断
		PLC_ACC_BIT|=1;
	}
}

static void no_amount_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1!=temp2)||(PLC_ACC_BIT&0X01))     
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dno_amount_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1!=temp2)||(PLC_ACC_BIT&0X01))     
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Dno_amount_and(void)	   //32位AND不等于比较指令
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1!=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void no_amount_and(void)	   //16位AND不等于比较指令
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1!=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}


static void LDP(void)	                        //LDP
{
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))	
	{ 
		PLC_ACC_BIT<<=1;
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))							                    //当前值判断
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);		     
	} 
	else  							                                                     //上升沿判断
	{ 
		if(PLC_STL_Status == 1)                                             //为STL状态区
		{  
			PLC_ACC_BIT<<=1;
			if((PLC_LD_BIT(0X2fff&*PLC_Addr))&&(PLC_BIT_TEST(PLC_STL_Addr)))  //当前值判断 				              
			PLC_ACC_BIT|=0x01,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);      //
		}	
		else
		{ 
			PLC_ACC_BIT<<=1;
			if(PLC_LD_BIT(0X2fff&*PLC_Addr))							                   //当前值判断
			PLC_ACC_BIT|=0x01,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);      //
		}
	} 
	PLC_Addr++;	    
}

static void LDF(void)	 //LDF
{ 
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))	 //上升沿判断
	{  
		if(PLC_STL_Status == 1)                       //为STL状态区 
		{  
			PLC_ACC_BIT<<=1;
			if((!(PLC_LD_BIT(0X2fff&*PLC_Addr)))&&(PLC_BIT_TEST(PLC_STL_Addr)))//当前值判断 				              
			PLC_ACC_BIT|=0x01,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);
		}	
		else
		{ 
			PLC_ACC_BIT<<=1;
			if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))                     //当前值判断
			PLC_ACC_BIT|=1,PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);//
		}		         
	}
	else
	{  
		PLC_ACC_BIT<<=1,PLC_ACC_BIT&=0XFE;        //清除输出标准开关
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))        //当前值判断
		PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);//
	}  
	PLC_Addr++;
}

static void ANDP(void)	 //ANDP
{ 
	u8  logic;
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))       //查出当前步号对应的逻辑值
	{ 
		logic=0;
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))   					   //当前值判断
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);         //
	}  
	else	 							                                   //上升沿判断
	{ 
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))	                 //当前值判断
		logic=1,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);  //上升沿成立
		else
		logic=0;		                                         //上升沿不成立
	}
	if((PLC_ACC_BIT&0x01)&&(logic==1))
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE;
	PLC_Addr++;	    
}

static void ANDF(void)	 //ANDF
{ 
	u8  logic;
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))			  //上升沿判断
	{ 
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))                //当前值判断
		logic=1,PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);  //
		else
		logic=0;		 //
	}
	else
	{
		logic=0;
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))                   //当前值判断
		PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);		        //
	}  
	if((PLC_ACC_BIT&0x01)&&(logic==1))
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE;
	PLC_Addr++;
} 

static void ORP(void)	 //ORP
{ 
	u8  logic;
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))							  
	{ 
		logic=0;                                           //
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))							   //当前值判断
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);		       //
	} 
	else                                                   //上升沿判断
	{ 
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))							     //当前值判断
		logic=1,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address); //
		else
		logic=0;		 //
	}	
	
	if(((PLC_ACC_BIT&0x01)==0x01)||(logic==1))
	PLC_ACC_BIT|=0x01;
	else
	PLC_ACC_BIT&=0XFE;
	PLC_Addr++;	    
}

static void ORF(void)	 //ORF
{ 
	u8  logic;
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))							                            //上升沿判断
	{ 
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))						    //当前值判断
		logic=1,PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);//
		else
		logic=0;		 //
	}
	else
	{  
		logic=0;
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))							   //当前值判断
		PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);	//
	}  
	if(((PLC_ACC_BIT&0x01)==0x01)||(logic==1))
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
	PLC_Addr++;
}

static void CJ_EX(u8 value)  //执行跳指令
{ 
	PLC_Addr++;
	if((*PLC_Addr&0xff00)==0x8000)
	{PLC_Addr=PLC_P_Addr[value/2],PLC_Addr++;}//取低位
}

static void CJ(void)
{ 
	if(PLC_ACC_BIT&0X01)
	{
		if((*PLC_Addr&0xff00)==0x8800) CJ_EX(*PLC_Addr); 
	}
	else PLC_Addr+=2;
}
 
static void CJP(void)	  //CJP
{ 
	if(PLC_LDP_TEST())    //上升沿判断
	{if((*PLC_Addr&0xff00)==0x8800) CJ_EX(*PLC_Addr);}
	else
	PLC_Addr+=2;		      //条件不满足执行跳过程序，减小CPU开销
}
 
static void SRET(void)
{ 
	u8 temp;
	PLC_ACC_BIT=process[0];	    //返回上一个逻辑状态值
	PLC_Addr=p_save[0];	  	    //返回上一个子程序前的执行地址
	for(temp=62;temp>0;temp--)
	{
		process[temp]=process[temp+1];    //data mov down
		p_save[temp]=p_save[temp+1]; 
	}      
}


static void P_MOV(void)
{ 
	u8 temp;
	for(temp=62;temp>0;temp--)
	{
		process[temp+1]=process[temp];    //数据 MOV up
		p_save[temp+1]=p_save[temp]; 
	}
	process[0]=PLC_ACC_BIT;	               //保存上一个逻辑状态值
	p_save[0]=PLC_Addr;				             //保存上一个子程序前的执行地址
}

static void CALL_EX(u8 value)
{ 
	PLC_Addr++;
	if((*PLC_Addr&0xff00)==0x8000)
	{P_MOV(),PLC_Addr=PLC_P_Addr[value/2];}//	先压入状态寄存器，以及前一个P指针的地址，
}

static void CALL(void)
{ 
	if(PLC_ACC_BIT&0X01)
	{ 
		if((*PLC_Addr&0xff00)==0x8800) 
		{CALL_EX(*PLC_Addr);}
	}
	else PLC_Addr+=2;
}

static void CALLP(void)	  //CALLP
{ 
	if(PLC_LDP_TEST())       //上升沿判断
	{if((*PLC_Addr&0xff00)==0x8800)CALL_EX(*PLC_Addr);}
	else
	PLC_Addr+=2;		        //条件不满足执行跳过程序，减小CPU开销 
}


void expand_SET(void)
{
  BIT_SET(0X2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_RST(void)
{
  RST(0X2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_OUT(void)
{
  OUT(0X2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_LD(void)
{
  LD(0X2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_LDI(void)
{
  LDI(0x2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_AND(void)
{
  AND(0x2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_ANI(void)
{
  ANI(0x2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_OR(void)
{
  OR(0x2FFF&*PLC_Addr);PLC_Addr++;
}
	
void expand_ORI(void)
{
  ORI(0x2FFF&*PLC_Addr);PLC_Addr++;
}	


static void enable_T_K(void)
{ 
	static u16 *p_data;
	T_value=*PLC_Addr%0x100;                //赋低8位值
	PLC_Addr++;
	T_value+=(*PLC_Addr%0x100)*0x100;       //赋高8位值
	p_data=PLC_16BIT+0x0900+T_number;       //指针指向T比较的值地址
	*p_data=T_value;                        //赋值给地址
	timer_enable(T_number);
	OUT(0X1600+(u8)T_number);
}

static void enable_T_D(void)
{
	PLC_16BIT[0x0900+T_number]=PLC_16BIT[0x1000+T_value];
	timer_enable(T_number);
	OUT(0X1600+(u8)T_number);
}

static void disable_T(void)
{
	timer_disble(T_number);
	OUT(0X1600+(u8)T_number);	 //disable T coil	
	OUT(0x0600+(u8)T_number);	 //reset T over coil
}

static void T_given_value_K(void)	      //
{
	if(PLC_ACC_BIT&0X01)  //
	enable_T_K();
	else
	PLC_Addr++,disable_T(); 
}
static void T_given_value_D(void)	      //
{ 
	T_value=(*PLC_Addr%0x100)/2;
	PLC_Addr++;
	switch(*PLC_Addr/0x100) 
	{ 
		case 0x86: T_value+=(*PLC_Addr%0x100)*0x80;        break;
		case 0x88: T_value+=(*PLC_Addr%0x100)*0x80+1000;   break; 
	}
	if((PLC_ACC_BIT&0X01)==0X01)  //是否有效
	enable_T_D();
	else
	disable_T();
}

static void operation_T(void)
{ 
	T_number=*PLC_Addr;       //将操作定时器的号码送入
	PLC_Addr++;				        //下一个功能取是K赋值还是D赋值
	switch(*PLC_Addr/0x100) 
	{ 
		case 0x80: T_given_value_K();              break;  //进行K赋值操作
		case 0x86: T_given_value_D();              break;  //进行D赋值操作
	}	
}

static void enable_C_K(void)	           //用常数K进行赋值
{
	u16 temp_bit,*p_C_enable_coil;u32 C;
	C_value=*PLC_Addr%0x100;                //赋低8位值
	PLC_Addr++;
	C_value+=(*PLC_Addr%0x100)*0x100;       //赋高8位值
	if(C_number>=0xC8)                      //判断是不是C200以上的 寄存器
	{  
		PLC_Addr++;
		C_value+=(*PLC_Addr%0x100)*0x10000;  //赋低8位值
		PLC_Addr++;
		C_value+=(*PLC_Addr%0x100)*0x1000000;//赋高8位值
		C=0x20001C00+(C_number-0xC8)*4; 
		temp_bit=1<<(C_number%0x10);
		if(PLC_RAM32(C)<C_value)             //把C当前值与目标值进行比较
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);//比较enable coil 
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			PLC_RAM32(C)+=1;
		}
		if(PLC_RAM32(C)<C_value)              //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
	}
	else
	{
		static u16 *p_data;
		p_data=PLC_16BIT+0x0500+C_number;      //
		temp_bit=1<<(C_number%0x10);
		if(*p_data<C_value)             
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);  //比较enable coil 
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			*p_data+=1;
		}
		if(*p_data<C_value)                   //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
	}
	OUT(0X2700+(u8)C_number);
}

static void enable_C_D(void)	    //用寄存器D进行赋值
{
	static u16 *p_data;
	u16 temp_bit,*p_C_enable_coil;u32 C;
	C_value=PLC_16BIT[0x1000+C_value];
	if(C_number>=0xC8)               //判断是不是C200以上的 寄存器
	{  		 
		C_value+=PLC_16BIT[0x1000+C_value+1]*0x10000;
		C=0x20001C00+(C_number-0xC8)*4; 
		temp_bit=1<<(C_number%0x10);
		if(PLC_RAM32(C)<C_value)      //把C当前值与目标值进行比较
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);   //比较enable coil
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			PLC_RAM32(C)+=1;
		}
		if(*p_data<C_value)            //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
		PLC_Addr+=2;                  
	}
	else
	{
		p_data=PLC_16BIT+0x0500+C_number;
		temp_bit=1<<(C_number%0x10);
		if(*p_data<C_value)    //把C当前值与目标值进行比较
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);   //比较enable coil
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			*p_data+=1;
		}
		if(*p_data<=C_value)  //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
	}
	OUT(0X2700+(u8)C_number);
}
 
static void disable_C_K(void)
{	
	u32 C;static u16 *p_data;
	C_value=*PLC_Addr%0x100;           //赋低8位值
	PLC_Addr++;
	C_value+=(*PLC_Addr%0x100)*0x100;  //赋高8位值
	if(C_number>=0xC8)                 //判断是不是C200以上的 寄存器
	{  
		PLC_Addr++;
		C_value=(*PLC_Addr%0x100)*0x10000;   //赋低8位值
		PLC_Addr++;
		C_value+=(*PLC_Addr%0x100)*0x1000000;//赋高8位值
		C=0x20001C00+(C_number-0xC8)*4; 
		if(PLC_RAM32(C)<C_value)              //把C当前值与目标值进行比较
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);        
	}
	else
	{
		p_data=PLC_16BIT+0x0500+C_number;
		if(*p_data<C_value)                 //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);       
	}
	OUT(0X2700+(u8)C_number);
}

static void disable_C_D(void)	     //关闭计数器C
{ 
	u32 C;static u16 *p_data;  
	if(C_number>=0xC8)               //判断是不是C200以上的 寄存器
	{  
		C_value=PLC_16BIT[0x1000+C_value];
		C_value+=PLC_16BIT[0x1000+C_value+1]*0x10000;
		C=0x20001C00+(C_number-0xC8)*4; 
		if(PLC_RAM32(C)<C_value)      //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
		PLC_Addr+=2;                  
	}
	else
	{
		C_value=PLC_16BIT[0x1000+C_value];
		p_data=PLC_16BIT+0x0500+C_number;
		if(*p_data<C_value)            //比较溢出值
		PLC_BIT_OFF(0x0E00+C_number);
		else
		PLC_BIT_ON(0x0E00+C_number);
	}	
	OUT(0X2700+(u8)C_number);
}

static void C_given_value_K(void)	      //程序中以K来设定值
{
	if((PLC_ACC_BIT&0X01)==0X01)        //
	enable_C_K();				                  //开启计数器
	else
	disable_C_K(); 
}

static void C_given_value_D(void)	      //程序中以D来设定值
{  
	C_value=(*PLC_Addr%0x100)/2;
	PLC_Addr++;
	switch(*PLC_Addr/0x100) 
	{ 
		case 0x86: C_value+=(*PLC_Addr%0x100)*0x80;        break;
		case 0x88: C_value+=(*PLC_Addr%0x100)*0x80+1000;   break; 
	}
	if(PLC_ACC_BIT&0X01)      //
	enable_C_D();
	else
	disable_C_D();
}
 
static void operation_C()
{
	C_number=*PLC_Addr;       //将操作计数器的号码送入
	PLC_Addr++;				        //下一个功能取是K赋值还是D赋值
	switch(*PLC_Addr/0x100) 
	{
		case 0x80: C_given_value_K();break;  //进行K赋值操作
		case 0x86: C_given_value_D();break;  //进行D赋值操作
	}	
}

static void FNC_AppInstruct(void) 
 { 
	 switch(*PLC_Addr) 
	{
		case 0x0002: PLC_Addr++,expand_OUT();              break;  //M1535以上的指令
		case 0x0003: PLC_Addr++,expand_SET();              break;  //M1535以上的指令
		case 0x0004: PLC_Addr++,expand_RST();              break;  //M1535以上的指令
		
		case 0x0005: PLC_Addr++,expand_OUT();              break;  //
		case 0x0006: PLC_Addr++,expand_SET();              break;  //
		case 0x0007: PLC_Addr++,expand_RST();              break;  //
		case 0x0008: PLC_Addr++,LPS();                     break;  //
		case 0x0009: PLC_Addr++,LPF();                     break;  //
		case 0x000C: PLC_Addr++,RST_T_C();                 break;  //执行RST C&T
		case 0x000D: PLC_Addr++,RST_D();                   break;  //执行D寄存器复位
		
		case 0x0010: PLC_Addr++,CJ();                      break;  //CJ  
		case 0x1010: PLC_Addr++,CJP();                     break;  //CJP  
		case 0x0012: PLC_Addr++,CALL();                    break;  //CALL
		case 0x1012: PLC_Addr++,CALLP();                   break;  //CALLP
		case 0x0014: PLC_Addr++,SRET();                    break;  //SRET			
		case 0x001C: PLC_Addr=PLC_Addr;                    break;  //FEND
		
		
	//	case 0X0020: PLC_Addr++,FOR();                     break;  //小小晟 20160929新增，FOR循环
	//  case 0X0022: PLC_Addr++,FOR_NEXT();                break;  //小小晟 20160929新增，FOR_NEST 循环结束
		case 0X0024: PLC_Addr++,CMP();                     break;  //16位比较传送指令
		case 0X1024: PLC_Addr++,CMPP();                    break;  //16位上升沿比较传送指令
		case 0X0025: PLC_Addr++,DCMP();                    break;  //32位比较传送指令
		case 0X1025: PLC_Addr++,DCMPP();                   break;  //32位上升沿比较传送指令
		case 0X0026: PLC_Addr++,ZCP();                     break;  //16位区间值比较传送指令
		case 0X0027: PLC_Addr++,DZCP();                    break;  //32位区间值比较传送指令
		case 0x0028: PLC_Addr++,MOV();		                 break;  //执行16bit传送指令
		case 0X0029: PLC_Addr++,DMOV();                    break;  //DMOV 
		case 0X002A: PLC_Addr++,SMOV();                    break;  //SMOV 	
		case 0X002C: PLC_Addr++,CML();                     break;  //CML取反指令
		case 0X002D: PLC_Addr++,DCML();                    break;  //DCML取反指令
		case 0X002E: PLC_Addr++,BMOV();                    break;  //成批传送
		case 0X0030: PLC_Addr++,FMOV();                    break;  //多点传送
		case 0X0031: PLC_Addr++,DFMOV();                   break;  //32位多点传送
		case 0X0032: PLC_Addr++,XCH();                     break;  //交换传送
		case 0X0033: PLC_Addr++,DXCH();                    break;  //32位交换传送
		case 0X0034: PLC_Addr++,BCD();                     break;  //二进制转换BCD
		case 0X0035: PLC_Addr++,DBCD();                    break;  //二进制转换DBCD
		case 0X0036: PLC_Addr++,BIN();                     break;  //二进制转换BIN
		case 0X0037: PLC_Addr++,DBIN();                    break;  //二进制转换DBIN
		
		case 0X0038: PLC_Addr++,ADD();					           break;  //加法指令
		case 0x0039: PLC_Addr++,DADD();                    break;  //DADD加法运算			
		case 0X003A: PLC_Addr++,SUB();					           break;  //减法指令
		case 0x003B: PLC_Addr++,DSUB();                    break;  //DSUB减法运算			
		case 0x003C: PLC_Addr++,MUL();                     break;  //MUL 乘法指令
		case 0x003D: PLC_Addr++,DMUL();                    break;  //DMUL乘法运算			
		case 0x003E: PLC_Addr++,DIV();                     break;  //DIV 乘法指令
		case 0x003F: PLC_Addr++,DDIV();                    break;  //DDIV除法运算			
		case 0x0040: PLC_Addr++,INC();                     break;  //16位逻辑运算加1指令
		case 0x1040: PLC_Addr++,INCP();                   break;  //16位上升沿逻辑运算加1指令
		case 0x0041: PLC_Addr++,DINC();                    break;  //32位逻辑运算加1指令
		case 0x1041: PLC_Addr++,DINC_P();                  break;  //32位上升沿逻辑运算加1指令
		case 0x0042: PLC_Addr++,DEC();                     break;  //16位逻辑运算减1指令
		case 0x1042: PLC_Addr++,DECP();                   break;  //16位上升沿逻辑运算减1指令
		case 0x0043: PLC_Addr++,DDEC();                    break;  //32位逻辑运算减1指令
		case 0x0044: PLC_Addr++,WAND();	                   break;  //逻辑运算与逻辑
		case 0x0045: PLC_Addr++,DWAND();	                 break;  //32位逻辑运算与逻辑
		case 0x0046: PLC_Addr++,WOR();                     break;  //逻辑运算或逻辑
		case 0x0047: PLC_Addr++,DWOR();                    break;  //32位逻辑运算或逻辑
		case 0x0048: PLC_Addr++,WXOR();                    break;  //逻辑运算异或逻辑
		case 0x0049: PLC_Addr++,DWXOR();                   break;  //32位逻辑运算异或逻辑
		case 0x004A: PLC_Addr++,NEG();                     break;  //逻辑运算取负数
		case 0x004B: PLC_Addr++,DNEG();                    break;  //32位逻辑运算取负数			
		case 0x004C: PLC_Addr++,ROR();                     break;  //ROR
		case 0x004D: PLC_Addr++,DROR();                    break;  //DROR
		case 0x004E: PLC_Addr++,ROL();                     break;  //ROL
		case 0x004F: PLC_Addr++,DROL();                    break;  //DROL
		case 0x0050: PLC_Addr++,RCR();                     break;  //RCR
		case 0x0051: PLC_Addr++,DRCR();                    break;  //DRCR
		case 0x0052: PLC_Addr++,RCL();                     break;  //RCL
		case 0x0053: PLC_Addr++,DRCL();                    break;  //DRCL
		
		
//		case 0x0054: PLC_Addr++,SFTR();                    break;  //SFTR
		
		case 0x0060: PLC_Addr++,ZRST();                    break; 
		case 0x0062: PLC_Addr++,DECO();                    break;  //整数转浮点    
		case 0x006A: PLC_Addr++,MEAN();                    break;	 //MEAN，求平均值指令		
		case 0x0070: PLC_Addr++,SQR();	                   break;  //SQR16位整数开方			
		case 0x0071: PLC_Addr++,DSQR();	                   break;  //SQR32位整数开方
		case 0x0072: PLC_Addr++,FLT();	                   break;  //16位整数转浮点
		case 0x0073: PLC_Addr++,DFLT();	                   break;  //32位整数转浮点	
		case 0x0076: PLC_Addr++,REFF();	                   break;  //REFF	
		case 0x0078: PLC_Addr++,MTR();	                   break;  //MTR
    case 0x007A: PLC_Addr++,HSCS();		                 break;  //高速计数置位  20160709
		
// 	case 0x0084: PLC_Addr++,PWM();                     break;  //PWM输出
		case 0x0082: PLC_Addr++,PLSY();                    break;  //高速脉冲输出
		case 0x0094: PLC_Addr++,ALT();	                   break;  //ALT
//	case 0x00B4: PLC_Addr++,ASCI();	                   break;  //ASCI
		case 0x00C0: PLC_Addr++,PID();	                   break;  //PID
		case 0x00ED: PLC_Addr++,ECMP();	                   break;  //ECMP
		case 0x00EE: PLC_Addr++,EZCP();	                   break;  //EZCP
		
    case 0x00F1: PLC_Addr++,DEMOV();                   break;  //
// 		case 0x00FD: PLC_Addr++,DEBCD();	                 break;  //DEBCD


		case 0x0101: PLC_Addr++,DEADD();                   break;  //浮点加法运算
		case 0x0103: PLC_Addr++,DESUB();	                 break;  //浮点减法运算
		case 0x0107: PLC_Addr++,DEDIV();	                 break;  //浮点乘法运算
		case 0x0105: PLC_Addr++,DEMUL();                   break;  //浮点除法运算
		case 0x010F: PLC_Addr++,DESQR();                   break;  //DESQR浮点开方
		case 0x0112: PLC_Addr++,INT();                     break;  //INT
		case 0x0113: PLC_Addr++,DINT();                    break;  //DINT
		case 0x0115: PLC_Addr++,DSIN();	                   break;  //DSIN
		case 0x0117: PLC_Addr++,DCOS();	                   break;  //DCOS
		case 0x0119: PLC_Addr++,DTAN();	                   break;  //DTAN			

		
		case 0x0136: PLC_Addr++,SWAP();                    break;  //SWAP
		case 0x0137: PLC_Addr++,DSWAP();                   break;  //DSWAP
		
		case 0x0150: PLC_Addr++,TCMP();	                   break;  //TCMP
		case 0x0152: PLC_Addr++,TZCP();	                   break;  //TZCP
		case 0x0154: PLC_Addr++,TADD();	                   break;  //TADD
		case 0x0156: PLC_Addr++,TSUB();	                   break;  //TSUB		
		case 0x015C: PLC_Addr++,TRD();	                   break;  //TRD	
		case 0x015E: PLC_Addr++,TWR();	                   break;  //TWR	
		case 0x0164: PLC_Addr++,GRY();	                   break;  //GRY
		case 0x0165: PLC_Addr++,DGRY();	                   break;  //DGRY
		case 0x0166: PLC_Addr++,GBIN();	                   break;  //GBIN
		case 0x0167: PLC_Addr++,DGBIN();	                 break;  //DGBIN
		
	
		case 0x01C2: PLC_Addr++,expand_LD();               break;  //M1535以上的指令
		case 0x01C3: PLC_Addr++,expand_LDI();              break;  //
		case 0x01C4: PLC_Addr++,expand_AND();              break;  //
		case 0x01C5: PLC_Addr++,expand_ANI();              break;  //
		case 0x01C6: PLC_Addr++,expand_OR();               break;  //
		case 0x01C7: PLC_Addr++,expand_ORI();              break;  //
		
		case 0x01CA: PLC_Addr++,LDP();			               break;  //上升延处理程序
		case 0x01CB: PLC_Addr++,LDF();			               break;  //上升延处理程序
		case 0x01CC: PLC_Addr++,ANDP();			               break;  //上升延处理程序
		case 0x01CD: PLC_Addr++,ANDF();			               break;  //上升延处理程序
		case 0x01CE: PLC_Addr++,ORP();			               break;  //上升延处理程序
		case 0x01CF: PLC_Addr++,ORF();			               break;  //上升延处理程序
		
		
		case 0X01D0: PLC_Addr++,amount();                  break;  //LD 16位等于比较
		case 0X01D1: PLC_Addr++,Damount();                 break;  //LD 32位等于比较
		case 0X01D2: PLC_Addr++,big();                     break;  //LD 16位大于比较
		case 0X01D3: PLC_Addr++,Dbig();                    break;  //LD 32位大于比较
		case 0X01D4: PLC_Addr++,less();                    break;  //LD 16位小于比较
		case 0X01D5: PLC_Addr++,Dless();                   break;  //LD 32位小于比较
		case 0X01D8: PLC_Addr++,no_amount();	             break;  //LD 16位不等于比较指令
		case 0X01D9: PLC_Addr++,Dno_amount();	             break;  //LD 32位不等于比较指令
		case 0X01DA: PLC_Addr++,less_amount();             break;  //LD 16位小于等于比较
		case 0X01DB: PLC_Addr++,Dless_amount();            break;  //LD 32位小于等于比较
		case 0X01DC: PLC_Addr++,big_amount();              break;  //LD 16位大于等于比较
		case 0X01DD: PLC_Addr++,Dbig_amount();             break;  //LD 32位大于等于比较
		
		case 0X01E0: PLC_Addr++,amount_and();              break;  //LD AND 16位等于比较
		case 0X01E1: PLC_Addr++,Damount_and();             break;  //LD AND 32位等于比较
		case 0X01E2: PLC_Addr++,big_and();                 break;  //LD AND 16位大于比较
		case 0X01E3: PLC_Addr++,Dbig_and();                break;  //LD AND 32位大于比较
		case 0X01E4: PLC_Addr++,less_and();                break;  //LD AND 16位小于比较
		case 0X01E5: PLC_Addr++,Dless_and();               break;  //LD AND 32位小于比较
		case 0X01E8: PLC_Addr++,no_amount_and(); 	         break;  //LD 16位不等于比较指令
		case 0X01E9: PLC_Addr++,Dno_amount_and(); 	       break;  //LD 32位不等于比较指令
		case 0X01EA: PLC_Addr++,less_amount_and();         break;  //LD AND 16位小于等于比较
		case 0X01EB: PLC_Addr++,Dless_amount_and();        break;  //LD AND 32位小于等于比较
		case 0X01EC: PLC_Addr++,big_amount_and();          break;  //LD AND 16位大于等于比较
		case 0X01ED: PLC_Addr++,Dbig_amount_and();         break;  //LD AND 32位大于等于比较
		
		case 0X01F0: PLC_Addr++,amount_OR();               break;  //LD OR 16位等于比较
		case 0X01F1: PLC_Addr++,Damount_OR();              break;  //LD OR 32位等于比较
		case 0X01F2: PLC_Addr++,big_OR();                  break;  //LD OR 16位大于比较
		case 0X01F3: PLC_Addr++,Dbig_OR();                 break;  //LD OR 32位大于比较
		case 0X01F4: PLC_Addr++,less_OR();                 break;  //LD OR 16位小于比较
		case 0X01F5: PLC_Addr++,Dless_OR();                break;  //LD OR 32位小于比较
		case 0X01F8: PLC_Addr++,no_amount_OR(); 	         break;  //LD 16位不等于比较指令
		case 0X01F9: PLC_Addr++,Dno_amount_OR(); 	         break;  //LD 32位不等于比较指令
		case 0X01FA: PLC_Addr++,less_amount_OR();          break;  //LD OR 16位小于等于比较
		case 0X01FB: PLC_Addr++,Dless_amount_OR();         break;  //LD OR 32位小于等于比较
		case 0X01FC: PLC_Addr++,big_amount_OR();           break;  //LD OR 16位大于等于比较
		case 0X01FD: PLC_Addr++,Dbig_amount_OR();          break;  //LD OR 32位大于等于比较					
		
		case 0x000F: PLC_Addr=PLC_Addr;                    break;  //如果遇到END指令则使后面结束
		case 0XF7FF: PLC_Addr++,RET();                     break;  //RET
		
		default:PLC_PROG_ERROR(M8065,02); PLC_Addr++;      break;  //遇到不支持的命令
	}
}
 
void find_p(void)//查找 P 所在的地址
{  
	u16 temp;
	PLC_Addr=PLC_START_Address;
	for(temp=0;temp<15999;temp++)//总共16000步
	{ 
		if((*PLC_Addr&0xFF00)==0xB000)
		PLC_P_Addr[*PLC_Addr%0x100]=PLC_Addr;
		PLC_Addr++;
	}
}

void RST_Y(void)
{PLC_16BIT[80]=PLC_16BIT[81]=0;}

u16 find_toend(void)//查找 P 所在的地址
{  
	u16 temp;
	PLC_Addr=PLC_START_Address-1;
	temp=0;	
	do{PLC_Addr++; temp++;}
	while((!(*PLC_Addr==0x000f))&&(temp<15998)); 	 
	return temp; 
}

// 小小晟，20160929优化
void PLC_ProInstructParse(void)
{ 
	static u8  puls,run_flag; 
  if(PLC_RUN)    //是否需要运行程序
  {
    if(run_flag == 1)
	  {
	     run_flag = 0;
		   PLC_8BIT(0X01E0)=0x09;		 
	  }
   if(PLC_8BIT(0X01E0)==0x09)			    //是否需要运行程序
   {
		PLC_BIT_ON(M8000);	              //运行强制M80000为ON
		PLC_BIT_OFF(M8001);	              //运行强制M80001为off
		run_Lamp;                         //亮起运行灯
		if(edit_prog==0x00)		            //判断是否存在程序编辑，如果编辑一次程序后必需重新计算P所在的址
		{ 
			find_p();
			edit_prog=1;
			if(find_toend()>15998)
			{  
				PLC_8BIT(0X01E0)=0x09;
				goto all_end;  
			}
		}
		
		if(puls==0x00)		                  //初始化脉冲用到8002 8003
		{
			PLC_BIT_ON(M8002);
			PLC_BIT_OFF(M8003);
		}
		
		PLC_Addr=PLC_START_Address;        //PLC到起始地址
		if(Write_Pro_flag == 0)
		{
		   PLC_IO_Refresh();               //刷新Y输出  
		} 
		
		do
		{
			switch(*PLC_Addr/0x100)          // 取高8位的数据
			{ 
				case 0x06: operation_T(),PLC_Addr++;                    break;  //operation all timer
				case 0x0E: operation_C(),PLC_Addr++;                    break;  //
				
				/* 操作S位元件所有的函数 */
				case 0x20: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x30: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x40: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x50: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x60: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x70: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  // 
				/* 操作S位元件所有的函数 */
				case 0x21: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x31: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x41: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x51: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x61: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x71: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* 操作S位元件所有的函数 */
				case 0x22: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x32: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x42: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x52: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x62: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x72: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* 操作S位元件所有的函数 */
				case 0x23: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x33: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x43: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x53: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x63: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x73: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* 操作X位元件所有的函数 */
				case 0x24: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x34: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x44: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x54: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x64: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x74: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* 操作Y位元件所有的函数 */
				case 0x25: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x35: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x45: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x55: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x65: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x75: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC5: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XD5: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XE5: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作T位元件所有的函数 */
				case 0x26: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x36: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x46: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x56: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x66: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x76: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC6: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作T位元件所有的函数 */
				case 0x27: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x37: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x47: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x57: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x67: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x77: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC7: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作M0_255位元件所有的函数 */
				case 0x28: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x38: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x48: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x58: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x68: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x78: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC8: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XD8: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XE8: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //	
				/* 操作M256_511位元件所有的函数 */
				case 0x29: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x39: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x49: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x59: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x69: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x79: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC9: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XD9: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XE9: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作M512_767位元件所有的函数 */
				case 0x2A: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3A: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4A: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5A: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6A: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7A: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCA: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDA: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEA: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作M768_1023位元件所有的函数 */
				case 0x2B: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3B: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4B: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5B: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6B: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7B: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCB: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDB: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEB: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作M1024_1279位元件所有的函数 */
				case 0x2C: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3C: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4C: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5C: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6C: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7C: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCC: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDC: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEC: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作M1280_1535位元件所有的函数 */
				case 0x2D: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3D: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4D: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5D: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6D: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7D: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCD: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDD: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XED: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* 操作C0-C255位元件所有的函数 */
				case 0x2E: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3E: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4E: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5E: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6E: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7E: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/*m8000-m8255*/
				case 0x2F: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3F: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4F: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5F: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6F: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7F: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCF: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDF: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEF: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/**********************STL步进模式***************************/
				case 0xF0: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //S
				case 0xF1: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0xF2: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0xF3: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				
				////////////////////////////////////////////////////////
        //基本逻辑指令二，如ANB、ORB、MPP、MRD、MPS、INV 等
				case 0XFF: 
				{
					other_function(*PLC_Addr);
				  PLC_Addr++;	      
				  break;   //MPP,MPS
				}
				case 0xB0: //指针P标识
				{
					PLC_Addr++;                                  
				  break;  
				}
				case 0x00: //遇到0X001C为FEND,0X000F为END指令   
        		{
					if(((*PLC_Addr%0x100)==0x1C)||((*PLC_Addr%0x100)==0x0F))
						goto all_end;
				}
				
				//////////////////////////////////////////////////////////////
				// 小小晟，20160929注释，应用指令				
				//////////////////////////////////////////////////////////////					
				default: //遇到不支持的命令 ,此处需要执行命令为16bit的指令
				{
					FNC_AppInstruct();                           
					break; 
				}
			}
		}while(1);
all_end: 
			D8010=D8011=D8012=PLC_RUN_TIME;       //保持扫描时间
			PLC_RUN_TIME=0;                       //清除扫描时间	
			puls=0x01;		 
			PLC_BIT_OFF(M8002),PLC_BIT_ON(M8003); //初始化脉冲用到8002 8003
	 }
	else
	{ 
		PLC_BIT_OFF(M8000);	                    //没有运行强制M80000为OFF
		PLC_BIT_ON(M8001);	                    //没有运行强制M80001为on
		D8012=0; 
		edit_prog=0;	                          //编程时要用到
		puls=0; 		                            //初始化脉冲用到8002 8003
		Stop_Lamp;                              //关闭运行灯,如果从运行状态切换到停止状态，需要清除Y输出
		if(Write_Pro_flag == 0)
		{
		   RST_Y(); 
		   PLC_IO_Refresh();                   //刷新Y输出  
		   RST_T_D_C_M_data();
		}  
		PLC_STL_CMD = PLC_STL_Status = 0;	     //上次程序中的步进	
	}
}
else		 
{

   	RST_Y();   												 //如果从运行状态切换到停止状态，需要清除Y输出                                               
		PLC_BIT_OFF(M8000);	               //没有运行强制M80000为OFF
		PLC_BIT_ON(M8001);	               //没有运行强制M80001为on
		D8012=0; 
		edit_prog=0;	                     //编程时要用到
		puls=0; 													 //初始化脉冲用到8002 8003		                                          
		Stop_Lamp;                         //关闭运行灯
		Write_Pro_flag = 0;
		PLC_IO_Refresh();

		PLC_STL_CMD = PLC_STL_Status = 0;	
	  if(run_flag == 0)
		{
        run_flag = 1;
				PLC_RUN_TIME=0;                      
        RST_T_D_C_M_data();
		}
		
}
	PLC_16BIT[0X701]=0X000;		           //设置版本号
} 


