/********************************************************/
// CPU��Ҫ��STM32F103--RAM�ڴ治С��64K	Flash�ڴ治С��128K
// ����������STM32F103RDT6��VET6����ͨ��
// �༭���ڣ�20150909
// editor by СС��
// ���꣺shop182385147.taobao.com
/********************************************************/

#include "stm32f10x.h"
#include <stdio.h>
#include "PLC_IO.h"
#include "PLC_Dialogue.h"
#include "math.h"          //��ѧ������ 
#include "Gray.h"          //������ת����
#include "PLC_CONF.H"

extern void RST_T_D_C_M_data(void);	 //ADD 20151214
extern unsigned char Y0P,Y1P;                //
extern unsigned short Plus_CMP0,Plus_CMP1;   //�����־λ
extern u8  X_DIY;	                           //�˲�ʱ��
extern u16 PLC_RUN_TIME;                     //ɨ��ʱ��
extern void RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//ʱ���޸ĳ���
extern void PLC_IO_Refresh(void);            //IOˢ���������
extern u16 PLC_16BIT[12100];                 //PLC_RAM���мĴ���
extern void timer_enable(u16 timer_number);	 //�˴��ڶ�ʱ��2���棬���ڿ�����ʱ������
extern void timer_disble(u16 timer_number);	 //�˴��ڶ�ʱ��2����     
static u8 PLC_ACC_BIT,PLC_MPS_BIT;           //����ִ��ר��(����ջ������ջ)
static const u16 *PLC_Addr;                  //PLC����ָ��
static const u16 *PLC_Err;                   //PLC����
static u8 T_number,C_number;                 //T&C��ַ����Ĵ���
static u16 T_value;                          //T�Ƚϻ���Ĵ���
static u16 C_value;                          //C�Ƚϻ���Ĵ���
static u32 mov_d_addr;                       //K?M&Y&S&Xָ���
static const u16 *PLC_P_Addr[129];	         //������ӳ���ȡָ��
static const u16 *p_save[129];               //���ӳ���ʱ������һ��ִ�е�λ
u8  Flag_bit=0xff,Transfer_bit,Transfer_bit1;//��ʵ����k�ı�־λ  ��С��������������CPU����     
u16 process[64];                             //���ӳ���ʱ������һ���ӳ���ֵ
u32 trade;                                   //�����ڼӼ�������С��������������CPU����     
u16 Transfer=0;                              //���ö�ش��ݺͳ������� ��С��������������CPU����  
u8 edit_prog;                                //���±�̻���Ĵ���
extern u8 Write_Pro_flag;
const unsigned char PLC_BIT_OR[]={0X01,0X02,0X04,0X08,0X10,0X20,0X40,0X80};
const unsigned char PLC_BIT_AND[]={0XFE,0XFD,0XFB,0XF7,0XEF,0XDF,0XBF,0X7F};
float_union FLOAT;
s32_union   u32data,u32data1;
u64_union   u64data,u64data2;

/***************************************************FOR**************************************************/
struct 
{
	const u16 *Addr[7];  //FOR ��ַ��¼
	u16 cycle[7];        //��ǰѭ���Ĵ���
	u16 count[7];        //Ŀ��ѭ���Ĵ���
	u8  point;           //forָ��ĵ���	
} FOR_CMD;



/***************************************************STL**************************************************/
static u16 PLC_STL_Addr;	  //STLָ���ַ��
static u8  PLC_STL_Status;    //STLָ�ǰ״̬ 0���׳���û��STL״̬������1ΪSTL��״̬��2ΪSTLֹͣ״̬ 
static u8  PLC_STL_CMD;		  //STL��־
static u8  PLC_STL_Count;     //������Ȧ����
static u16 PLC_STL_Coil[256]; //��Ȧ����Ĵ���
/********************************************************************************************************/
static u8 PLC_PL_BIT_TEST(u16 x){return((Step_Address[(x)/8] & PLC_BIT_OR[(x)%8])) ? (1) : (0);}
static u8 PLC_LD_BIT(u16 x){return((PLC_RAM8(RAM_ADDR+((x)/8)) & PLC_BIT_OR[(x)%8])) ? (1) : (0) ;}

static u8 PLC_LDP_TEST(void)	                        //�鿴�ǲ�������
{ 
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)==off)//�������ж�
	{ 
		   if(PLC_ACC_BIT&0X01)			                      //��ǰֵ�ж�
	     {
				 PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);   //
         return 1;
	     }
			 else return 0;
	 }
	 else
	 {
			if(!(PLC_ACC_BIT&0x01))						             //��ǰֵ�ж�
		  PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);    //
			return 0;
	 } 
}

/*********************************************
  �������ܣ�PLC������������
  err_id=01:ָ�����(δʶ��ָ��)
  err_id=02:ָ�����(�ݲ�֧��ָ��)
  err_id=10:���ݳ���(�޷�ʶ����������)
  err_id=11:���ݳ���(���ݶ�ȡ��ַ����)
  err_id=12:���ݳ���(��ַZ��ַδ֪)
  err_id=13:���ݳ���(��ַZ��ַ����)
  err_id=20:CJָ���ַ����
  D8061,M8061=PCӲ������
  D8063,M8063=����,ͨ�Ŵ���
  D8064,M8064=��������
  D8065,M8065=�﷨����
  D8066,M8066=��·����
  D8067,M8067=�������
  D8068,M8068=�����������
***********************************************/
void PLC_PROG_ERROR(u16 err,u16 err_id)
{
// PLC_BIT_ON(err);                              //�����־  СС�� ����
 D8012=0;	                                     //ɨ��ʱ��
 if (D8068==0)D8067=err_id;                      //�﷨����
 if (D8068==0)D8068=(PLC_Err-(u16*)(0x800605D)); //�������PC��
 D8069=D8068;
}

static void LD(u16 start_addr)	 //��ʼ��ַ����Ԫ�����ֵ
{ 
	 if(PLC_STL_Status == 1)                       //ΪSTL״̬��  ȫ�ֲ���
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
	if(PLC_STL_Status == 1)                          //ΪSTL״̬��  ȫ�ֲ���
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
   if (PLC_STL_CMD == 1)                 //�ж��ǲ��ǽ��벽��ģʽ
	 {
      if (PLC_STL_Status == 1)           //�ǲ��Ǵ�STLģʽ
			{
				 if(start_addr < 0X000A)//�жϿ�ʼ���� S000-S009
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
								  PLC_STL_Coil[PLC_STL_Count++]=start_addr;//��¼������ON��Ȧ��ַ λ�¸��������������
								}       
				        else 
 								  PLC_BIT_OFF(start_addr);      //OFF 
						 }

				 }
     }
     else
		 {
				 if(start_addr < 0X000A)//�жϿ�ʼ���� S000-S009
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


static void BIT_SET(u16 start_addr)//λ����
{ 
	u8 temp;
	if(PLC_ACC_BIT&0x01)
	{
		if (PLC_STL_Status == 1)       //ΪSTL״̬��
		{
			for(temp=0;temp<=PLC_STL_Count;temp++)
			PLC_BIT_OFF(PLC_STL_Coil[temp]);    //����ϴ�ON��Ȧ״̬
			
			PLC_BIT_OFF(PLC_STL_Addr); //OFF
			PLC_BIT_ON(start_addr);    //ON
			PLC_STL_Count=0;           //����ϴμ�¼ON��Ȧ����
		}
		else PLC_BIT_ON(start_addr);   //0N
	}
}


static void RST(u16 start_addr)//��λλ
{ 
  if((PLC_ACC_BIT&0X01)==0X01)
  PLC_BIT_OFF(start_addr);     //OFF 
}

static void RET(void)
{  
  PLC_STL_Status =0;               //�˳�����ģʽ �ó����������ͼ
}

void STL(u16 start_addr)	      //���� ģʽ
{
	PLC_STL_CMD = 1;            //ȫ�ֳ������ò�����־
	PLC_STL_Status = 1;         //��������ģʽ
	PLC_STL_Addr = start_addr;  //��¼������ַ
	PLC_ACC_BIT<<=1;
	if(PLC_BIT_TEST(PLC_STL_Addr))     
	PLC_ACC_BIT |=0x01;
}

// СС�ɣ�20160926�Ż�
static void other_function(u8 process_addr)
{
	 switch(process_addr)
   { 
		case 0xF8: //�鴮�� ANB 
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT >> 1)   & ((PLC_ACC_BIT & 0x01)|0xFE);           
			break;  
		}
		case 0xF9: //�鲢�� ORB
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT >> 1)   | (PLC_ACC_BIT & 0x01);                  
			break;  
	  }
		case 0xFA: //��ջ   MPS
    {			
			PLC_MPS_BIT = (PLC_MPS_BIT << 1)   | (PLC_ACC_BIT & 0x01);                  
			break;  
		}
		case 0xFB: //��ջ   MRD
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT & 0xfe) | (PLC_MPS_BIT & 0x01);                  
			break;	
		}
		case 0xFC: //��ջ   MPP
    {			
			PLC_ACC_BIT = (PLC_ACC_BIT & 0xfe) | (PLC_MPS_BIT & 0x01),PLC_MPS_BIT >>= 1;
			break;  
		}
		case 0xFD: // ȡ��   INV  
		{
			PLC_ACC_BIT = (PLC_ACC_BIT & 0xfe) | (~PLC_ACC_BIT & 0x01);                 
			break;
		}
		case 0xFF: //ȡ��   POP  
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


static void LPS(void)              //M1536~M3071λLPSָ���
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

static void LPF(void)              //M1536~M3071λLPSָ���
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


static void RESET_T(u8 process_addr) //��ʱ����λ
{  
	if(PLC_ACC_BIT&0x01) 			         //��ǰֵ�Ƿ���Ч
	{
		PLC_BIT_OFF(0x600+process_addr); //�����Ȧ
		PLC_BIT_OFF(0x2600+process_addr);//ʹ����Ȧ
		PLC_BIT_ON(0x2300+process_addr); //��λ��Ȧ
		PLC_16BIT[0x0800+process_addr]=0;//ʵ�ʼ�����
	}
	else
	PLC_BIT_OFF(0x2300+process_addr);   //��λ��Ȧ
}

static void RESET_C(u8 process_addr) 	              //��ʱ����λ
{ 
	static u16 *p_data;
	if((PLC_ACC_BIT&0x01)==0x01) 		                  //��ǰֵ�Ƿ���Ч
	{ 
		if((process_addr>=0XC8)&&(process_addr<=0XFF))
		{
			p_data=PLC_16BIT+0x0500+process_addr;         //ָ��ֵ��ַ
			*p_data=0;                                    //�����ַ ��λ
			p_data+=1;				                             //��Ϊ��32λ
			*p_data=0;                                    //�����ַ ��λ
			PLC_BIT_OFF(0x00E0+process_addr);             //ָ�������Ȧ���������Ȧ 
		}
		else
		{
			p_data=PLC_16BIT+0x0500+process_addr;           //ָ��ֵ��ַ
			*p_data=0;							                        //�����ַ
			PLC_BIT_OFF(0x00E0+process_addr);              //ָ�������Ȧ���������Ȧ               
		}
	}
	OUT(0X3700+process_addr);
}

static void RST_T_C(void)                            //����T_CλRSTָ���
{  
	switch(*PLC_Addr/0x100)
	{
		case 0x86: RESET_T(*PLC_Addr),PLC_Addr++;break;//��λT
		case 0x8E: RESET_C(*PLC_Addr),PLC_Addr++;break;//��λC
	}
}

 
static void MOV_TO_K_H(u8 i,u32 data,u8 addr)//���� MOV ?? K?X&Y&S&M ��ַ����
{  
	u8 LL_BIT;                         //��Ҫ�����ƶ�����λ�üĴ���
	u16 JOB_ADDR;
	int64_t MOV_DATA_64BIT,MOV_DATA_64BIT_BACKUP,MOV_DATA_BACKUP1;  //�ƶ�32λ����
	mov_d_addr|=addr<<8; 
	mov_d_addr+=Transfer;                      //�������ݺͶ��ʱ Transfer��������ƽʱ����0
	LL_BIT=mov_d_addr%0x20;						         //��Ҫ�ƶ�����λ
	JOB_ADDR=(mov_d_addr/0x20)*4;              //���ڵ���ʼ��ַ  
	switch(i)							 
	{            //	  �ƶ�λ������Ҫ���������ô���					  �������ƶ���Ҫ��λ��			 ����Ҫ������ȡ����
		case 0x82: MOV_DATA_64BIT_BACKUP=data&0X0000000F,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X0000000F<<LL_BIT); break;//����K1��С
		case 0x84: MOV_DATA_64BIT_BACKUP=data&0X000000FF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X000000FF<<LL_BIT); break;//����K2��С
		case 0x86: MOV_DATA_64BIT_BACKUP=data&0X00000FFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X00000FFF<<LL_BIT); break;//����K3��С
		case 0x88: MOV_DATA_64BIT_BACKUP=data&0X0000FFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X0000FFFF<<LL_BIT); break;//����K4��С
		case 0x8A: MOV_DATA_64BIT_BACKUP=data&0X000FFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X000FFFFF<<LL_BIT); break;//����K5��С
		case 0x8C: MOV_DATA_64BIT_BACKUP=data&0X00FFFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X00FFFFFF<<LL_BIT); break;//����K6��С
		case 0x8E: MOV_DATA_64BIT_BACKUP=data&0X0FFFFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0X0FFFFFFF<<LL_BIT); break;//����K7��С
		case 0x90: MOV_DATA_64BIT_BACKUP=data&0XFFFFFFFF,MOV_DATA_64BIT_BACKUP<<=LL_BIT,MOV_DATA_BACKUP1=~(0XFFFFFFFF<<LL_BIT); break;//����K8��С
		default:   PLC_Addr+=3;                           break;  //������֧�ֵ�����
	}                                          
	MOV_DATA_64BIT=PLC_RAM64(RAM_ADDR+JOB_ADDR);
	MOV_DATA_64BIT&=MOV_DATA_BACKUP1;             //����Ҫ���͵�λ����� 
	MOV_DATA_64BIT|=MOV_DATA_64BIT_BACKUP;        //������Ҫ����λ��   
	PLC_RAM64(RAM_ADDR+JOB_ADDR)=MOV_DATA_64BIT;  //�����ݴ���Ŀ��λ��
}
 

static signed int MOV_K(u8 Addr)	              //����K?X&Y&S&M����
{ 
	static u16 LL_BIT,JOB_ADDR;                   //��Ҫ�����ƶ�����λ�üĴ���
	static uint64_t MOV_DATA_64BIT;               //�ƶ�64λ����
	mov_d_addr|=(Addr<<8);                    
	mov_d_addr+=Transfer;                         //�������ݺͶ��ʱ Transfer��������ƽʱ����0
	LL_BIT=mov_d_addr%0x20;						            //��Ҫ�ƶ�����λ
	JOB_ADDR=(mov_d_addr/0x20)*4;                 //���ڵ���ʼ��ַ                 
	MOV_DATA_64BIT=PLC_RAM64(RAM_ADDR+JOB_ADDR),
	MOV_DATA_64BIT>>=LL_BIT; 									 
	return  (signed int)MOV_DATA_64BIT;
}

//�������λ��ַ
u16 D_C_T_addr(u8 l_value)
{ 
	static u16 temp; 
	switch(*PLC_Addr/0x100)
	{
		case 0x80: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0700+temp/2,PLC_Addr++;      break;//���ڵ���D1000
		case 0x82: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0800+temp/2,PLC_Addr++;      break;//���T�ĵ�ַ������ַ��Ϊֵ
		case 0x84: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x0500+temp/2,PLC_Addr++;      break;//���C�ĵ�ַ������ַ��Ϊֵ
		case 0x86: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x1000+temp/2,PLC_Addr++;      break;//���D�ĵ�ַ������ַ��Ϊֵ
		case 0x88: temp=l_value+((*PLC_Addr%0x100)*0x100),temp=0x1000+temp/2+1000,PLC_Addr++; break;//���ڵ���D1000
	}
	return temp;
}

//=======================================================================================================
// ��������:  static u16 addr_value(void)
// ���������� ����PLC��ַ��k��ʵ��
// �䡡��:  void      
// �䡡��:  ��ַ������     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��			  
// �ա���:  2015��6��2��
// ��  ע:  
//-------------------------------------------------------------------------------------------------------
// �޸���:
// �ա���:
// ��  ע: �Ż��������������в���ָ�����  �Ż��󷵻ص�ַ k������
//-------------------------------------------------------------------------------------------------------
//=======================================================================================================
static u16 addr_value(void)                                        
{  
	static u8 temp;static u16 temp1;
	switch(*PLC_Addr/0x100)
	{
		case 0x84: temp=*PLC_Addr,PLC_Addr++,temp1=*PLC_Addr<<8|temp,PLC_Addr++,Flag_bit=0;break;//������ K4M0 ֮��Ĵ���                                              break;//����C�ĵ�ַ
		case 0x86: temp=*PLC_Addr,PLC_Addr++;temp1=D_C_T_addr(temp);                       break;//���D��C��T�ĵ�ַ          
	}
	return temp1;
}



//=======================================================================================================
// ��������:  static u32 addr_value_prog(void)   
// ���������� ����PLC��ַ��k��ʵ��
// �䡡��:  void      
// �䡡��:  ��ַ������     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��23��
// ��  ע:  
//-------------------------------------------------------------------------------------------------------
// �޸���:
// �ա���:
// ��  ע: 
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
	
	if(Type_F == 0x8680)      temp=RAM_D8000_ADDR+temp2,                      PLC_Addr++;//���D�ĵ�ַ D8000
	else if(Type_F == 0x8682) temp=RAM_T_ADDR+temp2,                      PLC_Addr++;//���T�ĵ�ַ 
	else if(Type_F == 0x8684) temp=RAM_C_ADDR+temp2,                      PLC_Addr++;//���C�ĵ�ַ
	else if(Type_F == 0x8686) temp=RAM_D_ADDR+temp2,                      PLC_Addr++;//���D�ĵ�ַ 
	else if(Type_F == 0x8688) temp=RAM_D1000_ADDR+temp2,                      PLC_Addr++;//���ڵ���D1000
	else if(Type_F == 0x8482) temp=MOV_K(*PLC_Addr)&0X0000000F,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x8484) temp=MOV_K(*PLC_Addr)&0X000000FF,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x8486) temp=MOV_K(*PLC_Addr)&0X00000FFF,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x8488) temp=MOV_K(*PLC_Addr)&0X0000FFFF,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x848A) temp=MOV_K(*PLC_Addr)&0X000FFFFF,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x848C) temp=MOV_K(*PLC_Addr)&0X00FFFFFF,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x848E) temp=MOV_K(*PLC_Addr)&0X0FFFFFFF,Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���    
	else if(Type_F == 0x8490) temp=MOV_K(*PLC_Addr),           Flag_bit=0,PLC_Addr++;//������ K4M0 ֮��Ĵ���  
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
// ��������:  static void target(void)
// ���������� �Ӽ������롱���򡱡���򡱹��ø�ֵ����	��DMOV��DADD��DSUB��ָ��Ľ�����ݳ�ȥ
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��23��
// ��  ע:  
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
		case 0x8482: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x8484: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x8486: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x8488: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x848A: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x848C: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x848E: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case 0x8490: MOV_TO_K_H(Type_F,trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���     
		
		case 0x8680: PLC_RAM32(RAM_D8000_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //���D�ĵ�ַ D8000
		case 0x8682: PLC_RAM32(RAM_T_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //���T�ĵ�ַ 
		case 0x8684: PLC_RAM32(RAM_C_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //���C�ĵ�ַ
		case 0x8686: PLC_RAM32(RAM_D_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //���D�ĵ�ַ 
		case 0x8688: PLC_RAM32(RAM_D1000_ADDR+temp2+Transfer)=trade,PLC_Addr++;break; //���ڵ���D1000
		/************************************************K1M0�Ĵ���"Z"*******************************************************************/
		case 0xA482: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0��V0-V3	
		case 0xA483: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K2M0�Ĵ���"Z"*******************************************************************/
		case 0xA484: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0��V0-V3	
		case 0xA485: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K3M0�Ĵ���"Z"*******************************************************************/
		case 0xA486: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0��V0-V3	
		case 0xA487: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K4M0�Ĵ���"Z"*******************************************************************/
		case 0xA488: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V0-V3
		case 0xA489: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V4-V7		
		/************************************************K5M0�Ĵ���"Z"*******************************************************************/
		case 0xA48A: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K5M0��V0-V3	
		case 0xA48B: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K5M0��V4-V7		
		/************************************************K6M0�Ĵ���"Z"*******************************************************************/
		case 0xA48C: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K6M0��V0-V3	
		case 0xA48D: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K6M0��V4-V7		
		/************************************************K7M0�Ĵ���"Z"*******************************************************************/
		case 0xA48E: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K7M0��V0-V3	
		case 0xA48F: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K7M0��V4-V7		
		/************************************************K8M0�Ĵ���"Z"*******************************************************************/
		case 0xA490: mov_d_addr=+DZ0_Z3(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//k8M0��V0-V4	
		case 0xA491: mov_d_addr=+DZ4_Z7(temp2),MOV_TO_K_H(Type_F,trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K8M0��V4-V7			
		/************************************************T�Ĵ���"Z"*******************************************************************/
		case 0xA682: PLC_RAM32(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���T�ĵ�ַ����Z0-Z3
		case 0xA683: PLC_RAM32(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���T�ĵ�ַ����Z4-Z7
		/************************************************C�Ĵ���"Z"*******************************************************************/
		case 0xA684: PLC_RAM32(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����Z0-Z3
		case 0xA685: PLC_RAM32(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����Z4-Z7
		/************************************************D�Ĵ���"Z"*******************************************************************/
		case 0xA686: PLC_RAM32(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���D�ĵ�ַ����Z0-Z3
		case 0xA687: PLC_RAM32(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���D�ĵ�ַ����Z4-Z7
		case 0xA688: PLC_RAM32(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���ڵ���D1000��Z0-Z3
		case 0xA689: PLC_RAM32(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���ڵ���D1000��Z4-Z7
 }
	 PLC_Addr+=2;
}

//=======================================================================================================
// ��������:  static void target(void)
// ���������� �Ӽ������롱���򡱡���򡱹��ø�ֵ����	��MOV��ADD��SUB��ָ��Ľ�����ݳ�ȥ
// �䡡��:  void      
// �䡡��:  void
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��23��
// ��  ע:  
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
		case  0x8482: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case  0x8484: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case  0x8486: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		case  0x8488: MOV_TO_K_H(Type_F,(u16)trade,*PLC_Addr),PLC_Addr++,Transfer_bit=1;break; //������ K4M0 ֮��Ĵ���    
		
		case  0x8680: PLC_RAM16(RAM_D8000_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //���D�ĵ�ַ D8000
		case  0x8682: PLC_RAM16(RAM_T_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //���T�ĵ�ַ 
		case  0x8684: PLC_RAM16(RAM_C_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //���C�ĵ�ַ
		case  0x8686: PLC_RAM16(RAM_D_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //���D�ĵ�ַ 
		case  0x8688: PLC_RAM16(RAM_D1000_ADDR+temp2+Transfer)=(u16)trade,PLC_Addr++;break; //���ڵ���D1000
		/************************************************K1M0�Ĵ���"V"*******************************************************************/
		case  0x9482: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0��V0-V3	
		case  0x9483: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K2M0�Ĵ���"V"*******************************************************************/
		case  0x9484: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0��V0-V3	
		case  0x9485: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K3M0�Ĵ���"V"*******************************************************************/
		case  0x9486: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0��V0-V3	
		case  0x9487: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K4M0�Ĵ���"V"*******************************************************************/
		case  0x9488: mov_d_addr=+V0_V3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V0-V3	
		case  0x9489: mov_d_addr=+V4_V7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V4-V7						
		/************************************************T�Ĵ���"V"*******************************************************************/
		case  0x9682: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���T�ĵ�ַ����V0-V3
		case  0x9683: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���T�ĵ�ַ����V4-V7
		/************************************************C�Ĵ���"V"*******************************************************************/
		case  0x9684: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����V0-V3
		case  0x9685: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����V4-V7
		/************************************************D�Ĵ���"V"*******************************************************************/
		case  0x9686: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���D�ĵ�ַ����V0-V3
		case  0x9687: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���D�ĵ�ַ����V4-V7
		case  0x9688: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���ڵ���D1000��V0-V3
		case  0x9689: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���ڵ���D1000��V4-V7
		
		/************************************************K1M0�Ĵ���"Z"*******************************************************************/
		case  0xA482: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0��V0-V3	
		case  0xA483: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K2M0�Ĵ���"Z"*******************************************************************/
		case  0xA484: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0��V0-V3	
		case  0xA485: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K3M0�Ĵ���"Z"*******************************************************************/
		case  0xA486: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0��V0-V3	
		case  0xA487: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K4M0�Ĵ���"Z"*******************************************************************/
		case  0xA488: mov_d_addr=+Z0_Z3(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V0-V3
		case  0xA489: mov_d_addr=+Z4_Z7(temp2),MOV_TO_K_H(Type_F,(u16)trade,PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V4-V7			
		/************************************************T�Ĵ���"Z"*******************************************************************/
		case  0xA682: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����Z0-Z3
		case  0xA683: PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����Z4-Z7
		/************************************************C�Ĵ���"Z"*******************************************************************/
		case  0xA684: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����Z0-Z3
		case  0xA685: PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���C�ĵ�ַ����Z4-Z7
		/************************************************D�Ĵ���"Z"*******************************************************************/
		case  0xA686: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���D�ĵ�ַ����Z0-Z3
		case  0xA687: PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���D�ĵ�ַ����Z4-Z7
		case  0xA688: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2)=(u16)trade,PLC_Addr++;break;//���ڵ���D1000��Z0-Z3
		case  0xA689: PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2)=(u16)trade,PLC_Addr++;break;//���ڵ���D1000��Z4-Z7
    }
}


//=======================================================================================================
// ��������:  static u16  cos_value(void)	
// ���������� 
// �䡡��:  void      
// �䡡��:  ���16λ����
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��19��
// ��  ע:  
//-------------------------------------------------------------------------------------------------------
// �޸���:
// �ա���:
// ��  ע: 
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
		case  0x8080: temp=temp2,                                      PLC_Addr++;break;  //���Kֵ
		case  0x8280: temp=temp2,                                      PLC_Addr++;break;  //���Hֵ
		case  0x8482: temp=MOV_K(*PLC_Addr)&0X0000000F,Transfer_bit1=1,PLC_Addr++;break;  //������ K4M0 ֮��Ĵ���    
		case  0x8484: temp=MOV_K(*PLC_Addr)&0X000000FF,Transfer_bit1=1,PLC_Addr++;break;  //������ K4M0 ֮��Ĵ���    
		case  0x8486: temp=MOV_K(*PLC_Addr)&0X00000FFF,Transfer_bit1=1,PLC_Addr++;break;  //������ K4M0 ֮��Ĵ���    
		case  0x8488: temp=MOV_K(*PLC_Addr)&0X0000FFFF,Transfer_bit1=1,PLC_Addr++;break;  //������ K4M0 ֮��Ĵ���    	
		case  0x8680: temp=PLC_RAM16(RAM_D8000_ADDR+temp2+Transfer),PLC_Addr++;break;         //���D�ĵ�ַ D8000
		case  0x8682: temp=PLC_RAM16(RAM_D_ADDR+temp2+Transfer),PLC_Addr++;break;         //���T�ĵ�ַ 
		case  0x8684: temp=PLC_RAM16(RAM_C_ADDR+temp2+Transfer),PLC_Addr++;break;         //���C�ĵ�ַ
		case  0x8686: temp=PLC_RAM16(RAM_D_ADDR+temp2+Transfer),PLC_Addr++;break;         //���D�ĵ�ַ 
		case  0x8688: temp=PLC_RAM16(RAM_D1000_ADDR+temp2+Transfer),PLC_Addr++;break;         //���ڵ���D1000
		 	/************************************************K�Ĵ���"V"*******************************************************************/
		case  0x9080: temp=temp2+D8029,PLC_Addr++;break;//���K�ĵ�ַ����V0
		case  0x9081: temp=temp2+D8183,PLC_Addr++;break;//���K�ĵ�ַ����V1
		case  0x9082: temp=temp2+D8185,PLC_Addr++;break;//���K�ĵ�ַ����V2
		case  0x9083: temp=temp2+D8187,PLC_Addr++;break;//���K�ĵ�ַ����V3
		case  0x9084: temp=temp2+D8189,PLC_Addr++;break;//���K�ĵ�ַ����V4
		case  0x9085: temp=temp2+D8191,PLC_Addr++;break;//���K�ĵ�ַ����V5
		case  0x9086: temp=temp2+D8193,PLC_Addr++;break;//���K�ĵ�ַ����V6
		case  0x9087: temp=temp2+D8195,PLC_Addr++;break;//���K�ĵ�ַ����V7
		case  0x9280: temp=temp2+D8029,PLC_Addr++;break;//���H�ĵ�ַ����V0
		case  0x9281: temp=temp2+D8183,PLC_Addr++;break;//���H�ĵ�ַ����V1
		case  0x9282: temp=temp2+D8185,PLC_Addr++;break;//���H�ĵ�ַ����V2
		case  0x9283: temp=temp2+D8187,PLC_Addr++;break;//���H�ĵ�ַ����V3
		case  0x9284: temp=temp2+D8189,PLC_Addr++;break;//���H�ĵ�ַ����V4
		case  0x9285: temp=temp2+D8191,PLC_Addr++;break;//���H�ĵ�ַ����V5
		case  0x9286: temp=temp2+D8193,PLC_Addr++;break;//���H�ĵ�ַ����V6
		case  0x9287: temp=temp2+D8195,PLC_Addr++;break;//���H�ĵ�ַ����V7 	
		/************************************************K1M0�Ĵ���"V"*******************************************************************/
		case  0x9482: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0��V0-V3	
		case  0x9483: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K2M0�Ĵ���"V"*******************************************************************/
		case  0x9484: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0��V0-V3	
		case  0x9485: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K3M0�Ĵ���"V"*******************************************************************/
		case  0x9486: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0��V0-V3	
		case  0x9487: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K4M0�Ĵ���"V"*******************************************************************/
		case  0x9488: mov_d_addr=+V0_V3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0��V0-V3	
		case  0x9489: mov_d_addr=+V4_V7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0��V4-V7						
		/************************************************T�Ĵ���"V"*******************************************************************/
		case  0x9682: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//���T�ĵ�ַ����V0-V3
		case  0x9683: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//���T�ĵ�ַ����V4-V7
		/************************************************C�Ĵ���"V"*******************************************************************/
		case  0x9684: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//���C�ĵ�ַ����V0-V3
		case  0x9685: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//���C�ĵ�ַ����V4-V7
		/************************************************D�Ĵ���"V"*******************************************************************/
		case  0x9686: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//���D�ĵ�ַ����V0-V3
		case  0x9687: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//���D�ĵ�ַ����V4-V7
		case  0x9688: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V0_V3(temp2)*2),PLC_Addr++;break;//���ڵ���D1000��V0-V3
		case  0x9689: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+V4_V7(temp2)*2),PLC_Addr++;break;//���ڵ���D1000��V4-V7
	
		case  0xA080: temp=temp2+D8028,PLC_Addr++;break;//���K�ĵ�ַ����Z0
		case  0xA081: temp=temp2+D8182,PLC_Addr++;break;//���K�ĵ�ַ����Z1
		case  0xA082: temp=temp2+D8184,PLC_Addr++;break;//���K�ĵ�ַ����Z2
		case  0xA083: temp=temp2+D8186,PLC_Addr++;break;//���K�ĵ�ַ����Z3
		case  0xA084: temp=temp2+D8188,PLC_Addr++;break;//���K�ĵ�ַ����Z4
		case  0xA085: temp=temp2+D8190,PLC_Addr++;break;//���K�ĵ�ַ����Z5
		case  0xA086: temp=temp2+D8192,PLC_Addr++;break;//���K�ĵ�ַ����Z6
		case  0xA087: temp=temp2+D8194,PLC_Addr++;break;//���K�ĵ�ַ����Z7 	
		case  0xA280: temp=temp2+D8028,PLC_Addr++;break;//���H�ĵ�ַ����Z0
		case  0xA281: temp=temp2+D8182,PLC_Addr++;break;//���H�ĵ�ַ����Z1
		case  0xA282: temp=temp2+D8184,PLC_Addr++;break;//���H�ĵ�ַ����Z2
		case  0xA283: temp=temp2+D8186,PLC_Addr++;break;//���H�ĵ�ַ����Z3
		case  0xA284: temp=temp2+D8188,PLC_Addr++;break;//���H�ĵ�ַ����Z4
		case  0xA285: temp=temp2+D8190,PLC_Addr++;break;//���H�ĵ�ַ����Z5
		case  0xA286: temp=temp2+D8192,PLC_Addr++;break;//���H�ĵ�ַ����Z6
		case  0xA287: temp=temp2+D8194,PLC_Addr++;break;//���H�ĵ�ַ����Z7 	
		/************************************************K1M0�Ĵ���"Z"*******************************************************************/
		case  0xA482: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0��V0-V3	
		case  0xA483: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K2M0�Ĵ���"Z"*******************************************************************/
		case  0xA484: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0��V0-V3	
		case  0xA485: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K3M0�Ĵ���"Z"*******************************************************************/
		case  0xA486: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0��V0-V3	
		case  0xA487: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K4M0�Ĵ���"Z"*******************************************************************/
		case  0xA488: mov_d_addr=+Z0_Z3(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0��V0-V3
		case  0xA489: mov_d_addr=+Z4_Z7(temp2),temp=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0��V4-V7		
		case  0xA682: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//���T�ĵ�ַ����Z0-Z3
		case  0xA683: temp=PLC_RAM16(RAM_T_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//���T�ĵ�ַ����Z4-Z7
		/************************************************C�Ĵ���"Z"*******************************************************************/
		case  0xA684: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//���C�ĵ�ַ����Z0-Z3
		case  0xA685: temp=PLC_RAM16(RAM_C_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//���C�ĵ�ַ����Z4-Z7
		/************************************************D�Ĵ���"Z"*******************************************************************/
		case  0xA686: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//���D�ĵ�ַ����Z0-Z3
		case  0xA687: temp=PLC_RAM16(RAM_D_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//���D�ĵ�ַ����Z4-Z7
		case  0xA688: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z0_Z3(temp2)*2),PLC_Addr++;break;//���ڵ���D1000��Z0-Z3
		case  0xA689: temp=PLC_RAM16(RAM_D1000_ADDR+PLC_D_C_T_addr(temp2)+Z4_Z7(temp2)*2),PLC_Addr++;break;//���ڵ���D1000��Z4-Z7
	}	
	return temp;
}
//=======================================================================================================
// ��������:  static u32 cos_u32_value(void) 
// ���������� 
// �䡡��:  void      
// �䡡��:  ���32λ����
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��19��
// ��  ע:  
//-------------------------------------------------------------------------------------------------------
// �޸���:
// �ա���:
// ��  ע: 
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
		case  0x8080: u32data.data1[0]=temp2,PLC_Addr++,u32data.data1[1]=cos_value(),PLC_Addr-=2;break;//���Kֵ
		case  0x8280: u32data.data1[0]=temp2,PLC_Addr++,u32data.data1[1]=cos_value(),PLC_Addr-=2;break;//���Hֵ
		case  0x8482: D_data=MOV_K(*PLC_Addr)&0X0000000F,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x8484: D_data=MOV_K(*PLC_Addr)&0X000000FF,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x8486: D_data=MOV_K(*PLC_Addr)&0X00000FFF,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x8488: D_data=MOV_K(*PLC_Addr)&0X0000FFFF,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x848A: D_data=MOV_K(*PLC_Addr)&0X000FFFFF,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x848C: D_data=MOV_K(*PLC_Addr)&0X00FFFFFF,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x848E: D_data=MOV_K(*PLC_Addr)&0X0FFFFFFF,Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���    
		case  0x8490: D_data=MOV_K(*PLC_Addr),           Transfer_bit1=1,PLC_Addr++;break;//������ K4M0 ֮��Ĵ���   
			
		case  0x8680: D_data=PLC_RAM32(RAM_D8000_ADDR+temp2),PLC_Addr++;break;                 //���D�ĵ�ַ D8000
		case  0x8682: D_data=PLC_RAM32(RAM_T_ADDR+temp2),PLC_Addr++;break;                 //���T�ĵ�ַ 
		case  0x8684: D_data=PLC_RAM32(RAM_C_ADDR+temp2),PLC_Addr++;break;                 //���C�ĵ�ַ
		case  0x8686: D_data=PLC_RAM32(RAM_D_ADDR+temp2),PLC_Addr++;break;                 //���D�ĵ�ַ 
		case  0x8688: D_data=PLC_RAM32(RAM_D1000_ADDR+temp2),PLC_Addr++;break;                 //���ڵ���D1000
		case  0xA080: u32data.data1[0]=temp2+D8028,PLC_Addr++,u32data.data1[1]=cos_value()+D8029,PLC_Addr-=2;break;//���K�ĵ�ַ����Z0
		case  0xA081: u32data.data1[0]=temp2+D8182,PLC_Addr++,u32data.data1[1]=cos_value()+D8183,PLC_Addr-=2;break;//���K�ĵ�ַ����Z1
		case  0xA082: u32data.data1[0]=temp2+D8184,PLC_Addr++,u32data.data1[1]=cos_value()+D8185,PLC_Addr-=2;break;//���K�ĵ�ַ����Z2
		case  0xA083: u32data.data1[0]=temp2+D8186,PLC_Addr++,u32data.data1[1]=cos_value()+D8187,PLC_Addr-=2;break;//���K�ĵ�ַ����Z3
		case  0xA084: u32data.data1[0]=temp2+D8188,PLC_Addr++,u32data.data1[1]=cos_value()+D8189,PLC_Addr-=2;break;//���K�ĵ�ַ����Z4
		case  0xA085: u32data.data1[0]=temp2+D8190,PLC_Addr++,u32data.data1[1]=cos_value()+D8191,PLC_Addr-=2;break;//���K�ĵ�ַ����Z5
		case  0xA086: u32data.data1[0]=temp2+D8192,PLC_Addr++,u32data.data1[1]=cos_value()+D8193,PLC_Addr-=2;break;//���K�ĵ�ַ����Z6
		case  0xA087: u32data.data1[0]=temp2+D8194,PLC_Addr++,u32data.data1[1]=cos_value()+D8195,PLC_Addr-=2;break;//���K�ĵ�ַ����Z7 
		case  0xA280: u32data.data1[0]=temp2+D8028,PLC_Addr++,u32data.data1[1]=cos_value()+D8029,PLC_Addr-=2;break;//���H�ĵ�ַ����Z0
		case  0xA281: u32data.data1[0]=temp2+D8182,PLC_Addr++,u32data.data1[1]=cos_value()+D8183,PLC_Addr-=2;break;//���H�ĵ�ַ����Z1
		case  0xA282: u32data.data1[0]=temp2+D8184,PLC_Addr++,u32data.data1[1]=cos_value()+D8185,PLC_Addr-=2;break;//���H�ĵ�ַ����Z2
		case  0xA283: u32data.data1[0]=temp2+D8186,PLC_Addr++,u32data.data1[1]=cos_value()+D8187,PLC_Addr-=2;break;//���H�ĵ�ַ����Z3
		case  0xA284: u32data.data1[0]=temp2+D8188,PLC_Addr++,u32data.data1[1]=cos_value()+D8189,PLC_Addr-=2;break;//���H�ĵ�ַ����Z4
		case  0xA285: u32data.data1[0]=temp2+D8190,PLC_Addr++,u32data.data1[1]=cos_value()+D8191,PLC_Addr-=2;break;//���H�ĵ�ַ����Z5
		case  0xA286: u32data.data1[0]=temp2+D8192,PLC_Addr++,u32data.data1[1]=cos_value()+D8193,PLC_Addr-=2;break;//���H�ĵ�ַ����Z6
		case  0xA287: u32data.data1[0]=temp2+D8194,PLC_Addr++,u32data.data1[1]=cos_value()+D8195,PLC_Addr-=2;break;//���H�ĵ�ַ����Z7 				
		/************************************************K1M0�Ĵ���"Z"*******************************************************************/
		case  0xA482: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0��V0-V3	
		case  0xA483: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000000F,PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K2M0�Ĵ���"Z"*******************************************************************/
		case  0xA484: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0��V0-V3	
		case  0xA485: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000000FF,PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K3M0�Ĵ���"Z"*******************************************************************/
		case  0xA486: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0��V0-V3	
		case  0xA487: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00000FFF,PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K4M0�Ĵ���"Z"*******************************************************************/
		case  0xA488: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0��V0-V3
		case  0xA489: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0000FFFF,PLC_Addr++;break;//K4M0��V4-V7		
		/************************************************K5M0�Ĵ���"Z"*******************************************************************/
		case  0xA48A: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000FFFFF,PLC_Addr++;break;//K1M0��V0-V3	
		case  0xA48B: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X000FFFFF,PLC_Addr++;break;//K1M0��V4-V7	
		/************************************************K6M0�Ĵ���"Z"*******************************************************************/
		case  0xA48C: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00FFFFFF,PLC_Addr++;break;//K2M0��V0-V3	
		case  0xA48D: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X00FFFFFF,PLC_Addr++;break;//K2M0��V4-V7		
		/************************************************K7M0�Ĵ���"Z"*******************************************************************/
		case  0xA48E: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0FFFFFFF,PLC_Addr++;break;//K3M0��V0-V3	
		case  0xA48F: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100)&0X0FFFFFFF,PLC_Addr++;break;//K3M0��V4-V7		
		/************************************************K8M0�Ĵ���"Z"*******************************************************************/
		case  0xA490: mov_d_addr=+DZ0_Z3(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V0-V3
		case  0xA491: mov_d_addr=+DZ4_Z7(temp2),D_data=MOV_K(PLC_D_C_T_addr(temp2)/0x100),PLC_Addr++;break;//K4M0��V4-V7	
		/************************************************T�Ĵ���"Z"*******************************************************************/
		case  0xA682: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_T_ADDR+temp),PLC_Addr++;}break;//���T�ĵ�ַ����Z0-Z3
		case  0xA683: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_T_ADDR+temp),PLC_Addr++;}break;//���T�ĵ�ַ����Z4-Z7
		/************************************************C�Ĵ���"Z"*******************************************************************/
		case  0xA684: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_C_ADDR+temp),PLC_Addr++;}break;//���C�ĵ�ַ����Z0-Z3
		case  0xA685: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=510) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_C_ADDR+temp),PLC_Addr++;}break;//���C�ĵ�ַ����Z4-Z7
		/************************************************D�Ĵ���"Z"*******************************************************************/
		case  0xA686: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=15998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D_ADDR+temp);PLC_Addr++;}break;//���D�ĵ�ַ����Z0-Z3
		case  0xA687: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=15998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D_ADDR+temp);PLC_Addr++;}break;//���D�ĵ�ַ����Z4-Z7
		case  0xA688: {temp=(PLC_D_C_T_addr(temp2)+DZ0_Z3(temp2)*2);if(temp>=13998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D1000_ADDR+temp),PLC_Addr++;}break;//���ڵ���D1000��Z0-Z3
		case  0xA689: {temp=(PLC_D_C_T_addr(temp2)+DZ4_Z7(temp2)*2);if(temp>=13998) PLC_PROG_ERROR(M8067,6706),D_data=0; else D_data=PLC_RAM32(RAM_D1000_ADDR+temp),PLC_Addr++;}break;//���ڵ���D1000��Z4-Z7			
	}
	  PLC_Addr+=2;
			return D_data;
}
//=======================================================================================================
// ��������: static float float_value(void)
// ���������� 
// �䡡��:  void      
// �䡡��:  ���float����
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��19��
// ��  ע:  
//-------------------------------------------------------------------------------------------------------
// �޸���:
// �ա���:
// ��  ע: 
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
	 if(Type_F == 0x8080)      u32data.data1[0]=temp1,u32data.data1[1]=temp2, FLOAT.DATA=(float)u32data.data, PLC_Addr++;//���Kֵ
	 else if(Type_F == 0x8280) u32data.data1[0]=temp1,u32data.data1[1]=temp2, FLOAT.DATA=(float)u32data.data, PLC_Addr++;//���Hֵ            
	 else if(Type_F == 0x8680) FLOAT.DATA=PLC_RAMfolta(RAM_D8000_ADDR+temp1),PLC_Addr++;             //���D�ĵ�ַ D8000
	 else if(Type_F == 0x8682) FLOAT.DATA=PLC_RAMfolta(RAM_T_ADDR+temp1),PLC_Addr++;                 //���T�ĵ�ַ 
	 else if(Type_F == 0x8684) FLOAT.DATA=PLC_RAMfolta(RAM_C_ADDR+temp1),PLC_Addr++;                 //���C�ĵ�ַ
	 else if(Type_F == 0x8686) FLOAT.DATA=PLC_RAMfolta(RAM_D_ADDR+temp1),PLC_Addr++;                 //���D�ĵ�ַ 
     else if(Type_F == 0x8688) FLOAT.DATA=PLC_RAMfolta(RAM_D1000_ADDR+temp1),PLC_Addr++;             //���ڵ���D1000
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
// ��������:  static void target(void)
// ���������� �Ӽ������롱���򡱡���򡱹��ø�ֵ����	��DEMOV��DEADD��DESUB��ָ��Ľ�����ݳ�ȥ
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��23��
// ��  ע:  
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
	s16 PVn;  // ����ֵ
	s16 SV;   // �趨Ŀ��Desired value
	s16 Ts;   // ȡ��ʱ��
	s32 Su;
	s16 KP; // P 
	s16 Ti; // I
	s16 KD; // D
	s16 TD; // ΢������
	u32 Addr,Addr1; // ��ַ��¼
	u32 csp;        // PID�ڲ������ַ
	if((PLC_ACC_BIT&0X01)==0X01)        //ĸ�߳�����
	{
		SV=PLC_RAM16(addr_value_prog());   // �趨ֵ
		PVn=PLC_RAM16(addr_value_prog());  // ��������
		Addr=addr_value_prog();            // ��ȡ������ʼ��ַ
		Addr1= addr_value_prog();
		Ts=PLC_RAM16(Addr);                // ȡ��ʱ��
		KP=PLC_RAM16(Addr+6);  // P
		Ti=PLC_RAM16(Addr+8);  // I
		KD=PLC_RAM16(Addr+10); // D
		TD=PLC_RAM16(Addr+12); // ΢������
		csp=Addr+14;           // �ڲ�������ʼ��ַ
		 
		PLC_RAM16(csp+14)=KP*((PLC_RAM16(csp)-PLC_RAM16(csp+2))+((Ts/Ti)*PLC_RAM16(csp))+PLC_RAM16(csp+10)); 
		
		PLC_RAM16(csp) = PLC_RAM16(csp+4) - SV;         // ���㱾��ƫ��ֵ    
		
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
	if(PLC_ACC_BIT&0X01)         //�ж����������
	trade=cos_value(),target();  
	else
	PLC_Addr+=4;		              //����������ִ���������򣬼�СCPU����
}

static void DMOV(void)	        //���������ݴ���
{  	
   if(PLC_ACC_BIT&0X01)
   trade=cos_u32_value(),D_target(); 
   else	 
   PLC_Addr+=8;		              //����������ִ���������򣬼�СCPU����
}

static void DEMOV(void)	        //���������ݴ���
{  	
   if(PLC_ACC_BIT&0X01)
   trade=float_value(),float_target(); 
   else	 
   PLC_Addr+=8;		              //����������ִ���������򣬼�СCPU����
}

//======================================================================================================
// ��������: static void ZRST(void) 
// ���������� ZRSTָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void DSQR(void)
// ���������� 32λ�������� DSQR
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
//=======================================================================================================
static void DSQR(void)
{
   if((PLC_ACC_BIT&0X01)==0X01)
   {
  		trade=(u32)sqrt((double)cos_u32_value());
	    target();	
   } 
}

static void HSCS(void)		//���ټ�����λ
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
// ��������:  static void SQR(void)
// ���������� 16λ����λ RCRָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void DRCR(void)
// ���������� 32λ����λ RCRָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void RCR(void)
// ���������� 16λ����λ RCRָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                       //û�ж�������4������
}
//======================================================================================================
// ��������:  static void DROL(void)
// ���������� 32λ����λ RCLָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void ROL(void)
// ���������� 16λ����λ RCLָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}
//======================================================================================================
// ��������:  static void DROR(void)
// ���������� 32λѭ����λ RORָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void ROR(void)
// ���������� 16λѭ����λ RORָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}

//======================================================================================================
// ��������:  static void DROL(void)
// ���������� 32λѭ������λ ROLָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void ROL(void)
// ���������� 16λѭ������λ ROLָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}
//======================================================================================================
// ��������:  static void DSWAP(void)	
// ���������� 32λ���½��� DSWAPָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void DGBIN(void)	
// ���������� 16λ���½��� DGBINָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��8��4��
// ��  ע:  
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
	else PLC_Addr+=8;                      //û�ж�������8������
}

//=======================================================================================================
// ��������:  static void GBIN(void)	
// ���������� 16λ���½��� GBINָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}
//=======================================================================================================
// ��������:  static void DGRY(void)	
// ���������� 16λ���½��� DGRYָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��8��4��
// ��  ע:  
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
	else PLC_Addr+=8;                      //û�ж�������8������
}

//=======================================================================================================
// ��������:  static void GRY(void)	
// ���������� 16λ���½��� GRYָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}

//=======================================================================================================
// ��������:  static void SWAP(void)	
// ���������� 16λ���½��� SWAPָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}

//=======================================================================================================
// ��������:  static void SFTR(void)	 
// ���������� SFTRָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��8��9��
// ��  ע:  
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
//	 else PLC_Addr+=8;                      //û�ж�������4������
//}



//=======================================================================================================
// ��������:  static void XCH(void)	 
// ���������� 16λ�������� XCHָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
	else PLC_Addr+=4;                      //û�ж�������4������
}
//=======================================================================================================
// ��������:  static void DFMOV(void)	 
// ���������� 32λ�������� DXCHָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
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
// ��������:  static void DFMOV(void)	 
// ���������� 32λ��㴫�� DFMOVָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
//=======================================================================================================
static void DFMOV(void)	               
{  	
	signed short int temp,i;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_u32_value();            //Ҫ���ݵ�����
		D_target();                       //�ѵ�һ�����ݳ�ȥ
		temp=cos_u32_value();             //            <<<-------------|
		PLC_Addr-=4;                      //PLC_Addr-=4��Ϊ�˵��ص����� |<<-----|
		for(i=1;i<temp;i++)               //                                    |
		{                                 //                                    |
			if(Transfer_bit==1)Transfer=i*32;//                                  |
			else Transfer=i*4;             //                                    |
			PLC_Addr-=4;D_target();        //PLC_Addr-=4��Ϊ�˴��ݳ�ȥҪ��λ��	  |
		}                                 //                                    |
		PLC_Addr+=2;Transfer=0;           //PLC_Addr+=2��Ϊ�������������ȥ��---|
	}
}

//=======================================================================================================
// ��������:  static void FMOV(void)	 
// ���������� 16λ��㴫�� FMOVָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
//=======================================================================================================
static void FMOV(void)	             
{  	
	signed short int temp,i;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_value();                //Ҫ���ݵ�����
		target();                         //�ѵ�һ�����ݳ�ȥ
		temp=cos_value();                 //            <<<-------------|
		PLC_Addr-=2;                      //PLC_Addr-=2��Ϊ�˵��ص����� |<<-----|
		for(i=1;i<temp;i++)               //                                    |
		{                                 //                                    |
			if(Transfer_bit==1)Transfer=i*16;//                                  |
			else Transfer=i*2;             //                                    |
			PLC_Addr-=2;target();          //PLC_Addr-=2��Ϊ�˴��ݳ�ȥҪ��λ��	  |
		}                                 //                                    |
		PLC_Addr+=2;Transfer=0;           //PLC_Addr+=2��Ϊ�������������ȥ��---|
	}
	else PLC_Addr+=6;                    //û�ж�������6������
}

//=======================================================================================================
// ��������:  static void FMOV(void)	 
// ���������� 16λ�������� BMOVָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��6��27��
// ��  ע:  
//=======================================================================================================
static void BMOV(void)	                 
{  	
	signed short int temp,i;
	if((PLC_ACC_BIT&0X01)==0X01)
		{
		trade=cos_value();                //Ҫ���ݵ�����
		target();                         //�ѵ�һ�����ݳ�ȥ
		temp=cos_value();
		PLC_Addr-=2;                        //<<<---------------------------------|
		for(i=1;i<temp;i++)                 //                                    |
		{                                   //                                    |
			if(Transfer_bit1==1)Transfer=i*16;//                                    |
			else Transfer=i*2;               //                                    |
			PLC_Addr-=4;                     //                 
			trade=cos_value();               //Ҫ���ݵ�����
			if(Transfer_bit==1)Transfer=i*16;
			else Transfer=i*2;
			target();                   //�ѵڴ��ݳ�ȥ
		}                                   //                                      |
		PLC_Addr+=2;Transfer=0;             //PLC_Addr+=2��Ϊ�������������ȥ��-----|
	}
	else PLC_Addr+=6;                      //����û���¾�����6������
}

static void DCML(void)	                 //32λ����ȡ��
{  	
	s32 temp1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		trade=~temp1;
		D_target();
	}
	else PLC_Addr+=8;                     //����8������
}

static void CML(void)	                    //����ȡ��
{  	
	signed short int temp1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		trade=~temp1;
		target();
	}
	else PLC_Addr+=4;                   //����4������
}


u16 bcd[4]={0x1,0x10,0x100,0x1000};
static void SMOV(void)	          //16λ�Ƚϴ���ָ��
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
	else PLC_Addr+=10;              //����10������
}

//======================================================================================================
// ��������: static void TZCP(void)
// ����������TZCPָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
//=======================================================================================================
static void TZCP(void)
{
	u16 h,min,s,temp,temp1,temp3,temp4,h1,min1,s1;
	if((PLC_ACC_BIT&0X01)==0X01)
	{	
		temp3=addr_value();               //S1 ����Ƚ�����ֵ
		h=PLC_16BIT[temp3];
		min=PLC_16BIT[temp3+1];
		s=PLC_16BIT[temp3+2];
		
		temp4=addr_value();               //S2 ����Ƚ�����ֵ
		h1=PLC_16BIT[temp4];
		min1=PLC_16BIT[temp4+1];
		s1=PLC_16BIT[temp4+2];
		
		temp=addr_value();                //S3 ʱ��Ƚ�ֵ
		
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

static void EZCP(void)	          //16λ�Ƚϴ���ָ��
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
	else PLC_Addr+=16;              //����16������
}

static void DZCP(void)	          //16λ�Ƚϴ���ָ��
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
	else PLC_Addr+=16;              //����16������
}

static void ZCP(void)	            //16λ�Ƚϴ���ָ��
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
	else PLC_Addr+=8;              //����8������
}
//======================================================================================================
// ��������: static void TCMP(void)
// ����������TCMPָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
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
// ��������: static void ECMP(void)
// ����������ECMPָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
//=======================================================================================================
static void ECMP(void)	          //����Ƚϴ���ָ��
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
	else PLC_Addr+=12;              //����12������
}

//======================================================================================================
// ��������: static void DCMP(void)
// ����������DCMPָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
//=======================================================================================================
static void DCMP(void)	          //32λ�Ƚϴ���ָ��
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
	else PLC_Addr+=12;              //����12������
}

static void DCMPP(void)	  
{ 
	signed short int temp3;
	static int temp1,temp2;
	if(PLC_LDP_TEST())        //�������ж�
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
	PLC_Addr+=12;		         //����������ִ���������򣬼�СCPU����
}
//======================================================================================================
// ��������: static void CMP(void)
// ����������CMPָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
//=======================================================================================================
static void CMP(void)	          //16λ�Ƚϴ���ָ��
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
	else PLC_Addr+=6;              //����6������
}
//======================================================================================================
// ��������: static void CMP_P(void)
// ����������CMPPָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��8��26��
// ��  ע:  
//=======================================================================================================
static void CMPP(void)	  
{ 
	signed short int temp1,temp2,temp3;
	if(PLC_LDP_TEST())                       //�������ж�
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
	PLC_Addr+=6;		                          //����������ִ���������򣬼�СCPU����
}


static void DINC(void)	         //32λ�߼����� ��1ָ��
{ 
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=(u32)cos_u32_value()+1;
		PLC_Addr-=4;
		D_target();
	}
	else PLC_Addr+=4;              //����4������
}

static void DINC_P(void)	  //CALLP
{ 
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address)==off)//�������ж�
	{ 
		if(PLC_ACC_BIT&0X01)			                      //��ǰֵ�ж�
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
		if(!((PLC_ACC_BIT&0x01)==0x01))						 //��ǰֵ�ж�
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);//
		PLC_Addr+=4;		                           //����������ִ���������򣬼�СCPU����
	} 
}

static void INC(void)	            //�߼����� ��1ָ��
{ 
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_value();
		PLC_Addr-=2;
		trade++;
		target();
	}
	else PLC_Addr+=2;              //����2������
}

static void INCP(void)	  //INCP
{ 
	if(PLC_LDP_TEST())//�������ж�
	{ 
		trade=cos_value();
		PLC_Addr-=2;
		trade++;
		target();
	}
	else
	{
		PLC_Addr+=2;		                            //����������ִ���������򣬼�СCPU����
	} 
}

static void DDEC(void)                             //32λ�߼����� ��1ָ��
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_u32_value()-1;
		PLC_Addr-=4;
		D_target();
	}
	else PLC_Addr+=4;              //����4������
}

static void DEC(void)             //�߼����� ��1ָ��
{
	if(PLC_ACC_BIT&0X01)
	{
		trade=cos_value()-1;
		PLC_Addr-=2;
		target();
	}
	else PLC_Addr+=2;              //����2������
}

static void DECP(void)	          //INCP
{ 
	if(PLC_LDP_TEST())              //�������ж�
	{ 
		trade=cos_value()-1;
		PLC_Addr-=2;
		target();
	}
	else
	PLC_Addr+=2;		                           //����������ִ���������򣬼�СCPU����
}

static void DNEG(void)                             //32λ�߼�����ȡ����
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=0-cos_u32_value();
		PLC_Addr-=4;
		D_target();		 
	}
	else PLC_Addr+=4;              //����4������
}

static void NEG(void)                             //�߼�����ȡ����
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=0-cos_value();
		PLC_Addr-=2;
		target();
	}
	else PLC_Addr+=2;              //����2������
}

static void DWAND(void)	                          //�߼����㡰�롱
{ 
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();;
		trade=temp1&temp2;
		D_target();
	}
	else PLC_Addr+=12;              //����12������
}

static void WAND(void)	                          //�߼����㡰�롱
{ 
	signed short int temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		trade=temp1&temp2;
		target();
	}
	else PLC_Addr+=6;              //����6������
}

static void DWOR(void)	                           //�߼����㡰��
{  
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();;
		trade=temp1|temp2;
		D_target();
	}
	else PLC_Addr+=12;              //����12������
}

static void WOR(void)	                           //�߼����㡰��
{  
	signed short int temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_value();
		temp2=cos_value();
		trade=temp1|temp2;
		target();
	}
	else PLC_Addr+=6;              //����6������
}

static void DWXOR(void)	                          //�߼����㡰���
{ 
	u32 temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1=cos_u32_value();
		temp2=cos_u32_value();
		trade=temp1^temp2;
		D_target();
	}
	else PLC_Addr+=12;              //����12������
}

static void WXOR(void)	                          //�߼����㡰���
{ 
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade=cos_value()^cos_value();
		target();
	}
	else PLC_Addr+=6;              //����6������
}
//======================================================================================================
// ��������: static void TADD(void)
// ����������TADDָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
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
		PLC_Addr+=2;        //Щ�������  
		temp2=cos_value();
		if(Flag_bit==0xff)  //�ǲ���K4M0֮��ļĴ���
		{
			for(temp=0;temp<temp2;temp++)
			{data+=PLC_RAM16(temp1+temp*2);}
			PLC_Addr-=4;        //�������
		}
		else
		{  
			data=(u16)temp1;
			PLC_Addr-=4;   //����
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
// ��������:  static void ADD(void)	 
// ���������� 16λ�������� ADDָ��  �ӷ�
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��8��4��
// ��  ע:  
//=======================================================================================================
static void ADD(void)	   
{ 
	if(PLC_ACC_BIT&0X01)
	{
		trade=cos_value()+cos_value();
		target();
	}
	else PLC_Addr+=6;              //����6������
}
//=======================================================================================================
// ��������:  static void ALT(void)	 
// ���������� 16λ�������� ALTָ��
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��8��4��
// ��  ע:  
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
	else PLC_Addr+=2;                      //û�ж�������2������
}

//======================================================================================================
// ��������: static void TRD(void)
// ����������TRDָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
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
		//��ȡ������ʱ���������
	}
	else PLC_Addr+=2;              //����2������
}

//======================================================================================================
// ��������: static void TWR(void)
// ����������TWRָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
//=======================================================================================================
static void TWR(void)
{
	u16 temp;
	if(PLC_ACC_BIT&0X01)
	{
		temp=addr_value();
		RTC_Set(PLC_16BIT[temp],PLC_16BIT[temp+1],PLC_16BIT[temp+2],PLC_16BIT[temp+3],PLC_16BIT[temp+4],PLC_16BIT[temp+5]);
		//д��������ʱ����
	}
	else PLC_Addr+=2;              //����2������
}

//======================================================================================================
// ��������: static void TSUB(void)
// ����������TSUBָ���
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2015��7��26��
// ��  ע:  
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
		//ʱ�������
	}
}

static void SUB(void)	   //����
{ 
	if(PLC_ACC_BIT&0X01)
	{
		PLC_Err=PLC_Addr;
		trade=cos_value()-cos_value();
		target();
	} 
	else PLC_Addr+=6;              //����6������
}

static void DBCD(void)	            //������ת��DBCD
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
	else PLC_Addr+=8;              //����4������
}

static void BCD(void)	            //������ת��BCD
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
	else PLC_Addr+=4;              //����4������
}

static void DBIN(void)	         //������ת��DBIN
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
	else PLC_Addr+=8;              //����4������
}

static void BIN(void)	            //������ת��BIN
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
	else PLC_Addr+=4;              //����4������
}
 
static void MUL(void)	 //�˷�
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
	else PLC_Addr+=6;              //����6������
}
 

static void DIV(void)	 //����
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
	else PLC_Addr+=6;              //����6������
}

static void DADD(void)
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		PLC_Err=PLC_Addr;
		trade=cos_u32_value()+cos_u32_value();
		D_target();
	}
	else PLC_Addr+=12;              //����12������
}

static void DSUB(void)
{
	if((PLC_ACC_BIT&0X01)==0X01)
	{  
		PLC_Err=PLC_Addr;
		trade=cos_u32_value()-cos_u32_value();
		D_target();
	}
	else PLC_Addr+=12;              //����12������
}

static void DMUL(void)    
{	 
	signed short int temp;
	if(PLC_ACC_BIT&0X01)
	{
		u64data.data=(int64_t)(cos_u32_value()*cos_u32_value()); //Щ���64λ��ͬ��
		temp=addr_value(); PLC_Addr+=2;               
		PLC_16BIT[temp] = u64data.data1[0];         //���
		PLC_16BIT[temp+1] = u64data.data1[1]; 
		PLC_16BIT[temp+2] = u64data.data1[2]; 
		PLC_16BIT[temp+3] = u64data.data1[3];	
	}
	else PLC_Addr+=12;              //����12������
}

static void DDIV(void)
{
	signed short int temp1,temp2,temp3;
	if(PLC_ACC_BIT&0X01)
	{
		temp1 = cos_u32_value();             
		temp2 = cos_u32_value();		
		
		u32data.data=temp1/temp2;                   //��
		u32data1.data=temp1%temp2;                  //����
		
		temp3=addr_value() ;PLC_Addr+=2;                         
		PLC_16BIT[temp3] = u32data.data1[0];         //���
		PLC_16BIT[temp3+1]=u32data.data1[1]; 
		PLC_16BIT[temp3+2]=u32data1.data1[0]; 
		PLC_16BIT[temp3+3]=u32data1.data1[1];	
	}
	else PLC_Addr+=12;              //����12������
}

static void DFLT(void)	                 //����ת����
{  	
	signed int temp1;
	if(PLC_ACC_BIT&0X01)
	{
		temp1=cos_u32_value();
		trade1=(float)temp1;					
		float_target();
	}
	else PLC_Addr+=8;              //����8������
}
//======================================================================================================
// ��������:  static void DESQR(void)
// ���������� ��������ƻ�����
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void DINT(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade=(u32)float_value();
		D_target();
	} 
	else PLC_Addr+=8;              //����8������
}
//======================================================================================================
// ��������:  static void INT(void)
// ���������� ��������ƻ�����
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void INT(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade=(u16)float_value();
		PLC_Addr-=2; 
		target();
	} 
	else PLC_Addr+=4;              //����4������
}
//======================================================================================================
// ��������:  static void FLT(void)
// ���������� ����ת���� FLT
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void FLT(void)	                
{  	
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)cos_value();			
		float_target();PLC_Addr-=2; 
	}
	else PLC_Addr+=4;              //����4������
}

//======================================================================================================
// ��������:  static void DTAN(void)
// ���������� �������� DTAN
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void DTAN(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)tan((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //����8������
}
//======================================================================================================
// ��������:  static void DCOS(void)
// ���������� �������� DCOS
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void DCOS(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)cos((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //����8������
}

//======================================================================================================
// ��������:  static void DSIN(void)
// ���������� ��������  DSIN
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void DSIN(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)sin((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //����8������
}
//======================================================================================================
// ��������:  static void DESQR(void)
// ���������� ���㿪������ DESQR
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void DESQR(void)
{
	if(PLC_ACC_BIT&0X01)
	{
		trade1=(float)sqrt((double)float_value());
		float_target();
	} 
	else PLC_Addr+=8;              //����8������
}

//======================================================================================================
// ��������:  static void DEADD(void)	
// ���������� ����ӷ����� ָ��DEADD
// �䡡��:  void      
// �䡡��:  void     
// ȫ�ֱ���:  
// ����ģ��: 
// ������:  СС��
// �ա���:  2014��6��27��
// ��  ע:  
//=======================================================================================================
static void DEADD(void)	 
{  
	if(PLC_ACC_BIT&0X01)
	{
		trade1=float_value()+ float_value(); 						
		float_target();
	}
	else PLC_Addr+=12;              //����12������
}


static void DESUB(void)	           //����������� 
{  
	float temp1,temp2;
	if(PLC_ACC_BIT&0X01)
	{
		temp1 = float_value();              
		temp2 = float_value(); 
		
		trade1=temp1-temp2;					
		float_target();
	}
	else PLC_Addr+=12;              //����12������
}



static void DEDIV(void)	           //����������� 
{  
	float temp1,temp2;
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		temp1 = float_value();              
		temp2 = float_value(); 
		
		trade1=temp1/temp2;					
		float_target();
	}
	else PLC_Addr+=12;              //����12������
}

static void DEMUL(void)	                //����˷�����
{  
	if((PLC_ACC_BIT&0X01)==0X01)
	{
		trade1=float_value()*float_value();					
		float_target();
	}
	else PLC_Addr+=12;              //����12������
}

u16 DE[16]={0x0001,0x0003,0x0007,0x000F,0X001F,0X003F,0X007F,0X00FF,0X01FF,0X03FF,0X07FF,0X0FFF,0X1FFF,0X3FFF,0X7FFF,0XFFFF};	
void DECO()
{
  u8 i,t=1;
	int temp1=cos_value();      //��ȡ������
	int temp2=addr_value();     
	int n=cos_value();          //����ԭ������  
	temp1&=DE[n-1];             //����
  for(i=0;i<n;i++)            //����2��n�η�
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
	if(PLC_ACC_BIT&0X01)               //�ж����������
	{ 
		temp=cos_value();               //����Ƶ��
		PUS_TOTAL=cos_value();          //�������
		addr=*PLC_Addr;                 //�����ַ
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
			//IO�ڳ�ʼ��
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;      //A7 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
			TIM_DeInit(TIM3);
			TIM_TimeBaseStructure.TIM_Period=sys;		 								 // �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) 
			// �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж� 
			TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);				     // ʱ��Ԥ��Ƶ�� 72M/72 
			TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		 // ������Ƶ 
			TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; // ���ϼ���ģʽ 
			TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
			TIM_ClearFlag(TIM3, TIM_FLAG_Update);							    	 // �������жϱ�־ 
			TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
			TIM_Cmd(TIM3, ENABLE);																		 // ����ʱ��                 
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);		 // �ȹرյȴ�ʹ��			      
			PLC_BIT_ON(M8147);                                        // M8147 
		}	
	}
	else  
	{  
		PLC_Addr+=4;
		addr=*PLC_Addr;          //�����ַ
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
// 		  PLC_Addr+=4;		                   //����4������ ����������ִ���������򣬼�СCPU����              
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
//    if(PLC_ACC_BIT&0X01)               //�ж����������
//    { 
// 		  temp=cos_value();               //����Ƶ��
// 	    PUS_TOTAL=cos_value();            //�������
// 		  addr=*PLC_Addr;                 //�����ַ
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

//        //ֻ��ʼ��һ��  Y0 
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
//             TIM_ITConfig(TIM5, TIM_IT_CC2, ENABLE);//���жϣ��ж���Ҫ���д���
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
//             TIM8->DIER  = 0x02;   // ʹ�� CC1 �ж�
//          }
//          // TIM1 counter enable 
//           TIM_Cmd(TIM8, ENABLE); 
//           PLC_BIT_ON(0X00E0,148);    //M8148
// 	   }
//    }
//    else  
//    {   
//   		PLC_Addr+=4;
//       addr=*PLC_Addr;          //�����ַ
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




static void Damount(void)	 //32λ���ڱȽ�ָ��
{  
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)  //ΪSTL״̬��
	{ 
		PLC_ACC_BIT<<=1;		
		if((temp1==temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))      
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1==temp2)     //��ǰֵ�ж�
		PLC_ACC_BIT|=1;
	}
}


static void amount(void)	 //16λ���ڱȽ�ָ��
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)     //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1==temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1==temp2) 					//��ǰֵ�ж�
		PLC_ACC_BIT|=1;
	}
}

static void amount_OR()
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1==temp2)||(PLC_ACC_BIT&0X01))  //��ǰֵ�ж�
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 

}

static void Damount_OR()
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1==temp2)||(PLC_ACC_BIT&0X01))  //��ǰֵ�ж�
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0xFE; 
}

static void Damount_and(void)	 //32λAND���ڱȽ�
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1==temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void amount_and(void)	 //16λAND���ڱȽ�
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1==temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dbig(void)		    //32λ���ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)                       //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))    
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>temp2) 						               //��ǰֵ�ж�  �·��޸�"<"�ĳ���">"
		PLC_ACC_BIT|=1;
	}
}

static void big(void)		     //16λ���ڱȽ�ָ��
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)                       //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))      
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>temp2) 						               //��ǰֵ�ж� �·��޸�"<"�ĳ���">"
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

static void Dbig_and(void)		//32λAND���ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1>temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void big_and(void)		//16λAND���ڱȽ�ָ��
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1>temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dless(void)	     //32λС�ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)                       //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))      
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<temp2) 						               //��ǰֵ�ж�
		PLC_ACC_BIT|=1;
	}
}

static void less(void)	     //С�ڱȽ�
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)                       //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))    
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<temp2) 						               //��ǰֵ�ж�
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

static void Dless_and(void)	   //32λANDС�ڱȽ�
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1<temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void less_and(void)	   //16λANDС�ڱȽ�
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1<temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dless_amount(void)	     //32λС�ڵ��ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)            //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<=temp2) 						    //��ǰֵ�ж�
		PLC_ACC_BIT|=1;
	}
}	

static void less_amount(void)	      //16λС�ڵ��ڱȽ�ָ��
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)          //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1<=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1<=temp2) 						               //��ǰֵ�ж�
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

static void Dless_amount_and(void)	   //32λANDС�ڵ��ڱȽ�
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1<=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void less_amount_and(void)	     //16λANDС�ڵ��ڱȽ�
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1<=temp2)&&(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=0X01;
	else
	PLC_ACC_BIT&=0XFE;
}

static void Dbig_amount(void)	     //32λ���ڵ��ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)            //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>=temp2) 						               //��ǰֵ�ж�
		PLC_ACC_BIT|=1;
	}
}

static void big_amount(void)	     //16λ���ڵ��ڱȽ�ָ��
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)            //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1>=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1>=temp2) 						               //��ǰֵ�ж�
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

static void Dbig_amount_and(void)	   //32λAND���ڵ��ڱȽ�
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1>=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void big_amount_and(void)	   //16λAND���ڵ��ڱȽ�
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if((temp1>=temp2)&&(PLC_ACC_BIT&0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void Dno_amount(void)	   //32λ�����ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if(PLC_STL_Status == 1)            //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1!=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1!=temp2) 					 //��ǰֵ�ж�
		PLC_ACC_BIT|=1;
	}
}

static void no_amount(void)	    //16λ�����ڱȽ�ָ��
{ 
	signed short int temp1,temp2;
	temp1=cos_value();
	temp2=cos_value();
	if(PLC_STL_Status == 1)            //ΪSTL״̬��
	{  
		PLC_ACC_BIT<<=1;		
		if((temp1!=temp2)&&(PLC_BIT_TEST(PLC_STL_Addr)))     
		PLC_ACC_BIT|=1;
	}	
	else
	{ 
		PLC_ACC_BIT<<=1;
		if(temp1!=temp2) 						               //��ǰֵ�ж�
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

static void Dno_amount_and(void)	   //32λAND�����ڱȽ�ָ��
{ 
	s32 temp1,temp2;
	temp1=cos_u32_value();
	temp2=cos_u32_value();
	if((temp1!=temp2)&&((PLC_ACC_BIT&0X01)==0X01)) 
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
}

static void no_amount_and(void)	   //16λAND�����ڱȽ�ָ��
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
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))							                    //��ǰֵ�ж�
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);		     
	} 
	else  							                                                     //�������ж�
	{ 
		if(PLC_STL_Status == 1)                                             //ΪSTL״̬��
		{  
			PLC_ACC_BIT<<=1;
			if((PLC_LD_BIT(0X2fff&*PLC_Addr))&&(PLC_BIT_TEST(PLC_STL_Addr)))  //��ǰֵ�ж� 				              
			PLC_ACC_BIT|=0x01,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);      //
		}	
		else
		{ 
			PLC_ACC_BIT<<=1;
			if(PLC_LD_BIT(0X2fff&*PLC_Addr))							                   //��ǰֵ�ж�
			PLC_ACC_BIT|=0x01,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);      //
		}
	} 
	PLC_Addr++;	    
}

static void LDF(void)	 //LDF
{ 
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))	 //�������ж�
	{  
		if(PLC_STL_Status == 1)                       //ΪSTL״̬�� 
		{  
			PLC_ACC_BIT<<=1;
			if((!(PLC_LD_BIT(0X2fff&*PLC_Addr)))&&(PLC_BIT_TEST(PLC_STL_Addr)))//��ǰֵ�ж� 				              
			PLC_ACC_BIT|=0x01,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);
		}	
		else
		{ 
			PLC_ACC_BIT<<=1;
			if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))                     //��ǰֵ�ж�
			PLC_ACC_BIT|=1,PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);//
		}		         
	}
	else
	{  
		PLC_ACC_BIT<<=1,PLC_ACC_BIT&=0XFE;        //��������׼����
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))        //��ǰֵ�ж�
		PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);//
	}  
	PLC_Addr++;
}

static void ANDP(void)	 //ANDP
{ 
	u8  logic;
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))       //�����ǰ���Ŷ�Ӧ���߼�ֵ
	{ 
		logic=0;
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))   					   //��ǰֵ�ж�
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);         //
	}  
	else	 							                                   //�������ж�
	{ 
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))	                 //��ǰֵ�ж�
		logic=1,PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);  //�����س���
		else
		logic=0;		                                         //�����ز�����
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
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))			  //�������ж�
	{ 
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))                //��ǰֵ�ж�
		logic=1,PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);  //
		else
		logic=0;		 //
	}
	else
	{
		logic=0;
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))                   //��ǰֵ�ж�
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
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))							   //��ǰֵ�ж�
		PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);		       //
	} 
	else                                                   //�������ж�
	{ 
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))							     //��ǰֵ�ж�
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
	if(PLC_PL_BIT_TEST(PLC_Addr-PLC_START_Address))							                            //�������ж�
	{ 
		if(!(PLC_LD_BIT(0X2fff&*PLC_Addr)))						    //��ǰֵ�ж�
		logic=1,PLC_PL_BIT_OFF(PLC_Addr-PLC_START_Address);//
		else
		logic=0;		 //
	}
	else
	{  
		logic=0;
		if(PLC_LD_BIT(0X2fff&*PLC_Addr))							   //��ǰֵ�ж�
		PLC_PL_BIT_ON(PLC_Addr-PLC_START_Address);	//
	}  
	if(((PLC_ACC_BIT&0x01)==0x01)||(logic==1))
	PLC_ACC_BIT|=1;
	else
	PLC_ACC_BIT&=~1;
	PLC_Addr++;
}

static void CJ_EX(u8 value)  //ִ����ָ��
{ 
	PLC_Addr++;
	if((*PLC_Addr&0xff00)==0x8000)
	{PLC_Addr=PLC_P_Addr[value/2],PLC_Addr++;}//ȡ��λ
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
	if(PLC_LDP_TEST())    //�������ж�
	{if((*PLC_Addr&0xff00)==0x8800) CJ_EX(*PLC_Addr);}
	else
	PLC_Addr+=2;		      //����������ִ���������򣬼�СCPU����
}
 
static void SRET(void)
{ 
	u8 temp;
	PLC_ACC_BIT=process[0];	    //������һ���߼�״ֵ̬
	PLC_Addr=p_save[0];	  	    //������һ���ӳ���ǰ��ִ�е�ַ
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
		process[temp+1]=process[temp];    //���� MOV up
		p_save[temp+1]=p_save[temp]; 
	}
	process[0]=PLC_ACC_BIT;	               //������һ���߼�״ֵ̬
	p_save[0]=PLC_Addr;				             //������һ���ӳ���ǰ��ִ�е�ַ
}

static void CALL_EX(u8 value)
{ 
	PLC_Addr++;
	if((*PLC_Addr&0xff00)==0x8000)
	{P_MOV(),PLC_Addr=PLC_P_Addr[value/2];}//	��ѹ��״̬�Ĵ������Լ�ǰһ��Pָ��ĵ�ַ��
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
	if(PLC_LDP_TEST())       //�������ж�
	{if((*PLC_Addr&0xff00)==0x8800)CALL_EX(*PLC_Addr);}
	else
	PLC_Addr+=2;		        //����������ִ���������򣬼�СCPU���� 
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
	T_value=*PLC_Addr%0x100;                //����8λֵ
	PLC_Addr++;
	T_value+=(*PLC_Addr%0x100)*0x100;       //����8λֵ
	p_data=PLC_16BIT+0x0900+T_number;       //ָ��ָ��T�Ƚϵ�ֵ��ַ
	*p_data=T_value;                        //��ֵ����ַ
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
	if((PLC_ACC_BIT&0X01)==0X01)  //�Ƿ���Ч
	enable_T_D();
	else
	disable_T();
}

static void operation_T(void)
{ 
	T_number=*PLC_Addr;       //��������ʱ���ĺ�������
	PLC_Addr++;				        //��һ������ȡ��K��ֵ����D��ֵ
	switch(*PLC_Addr/0x100) 
	{ 
		case 0x80: T_given_value_K();              break;  //����K��ֵ����
		case 0x86: T_given_value_D();              break;  //����D��ֵ����
	}	
}

static void enable_C_K(void)	           //�ó���K���и�ֵ
{
	u16 temp_bit,*p_C_enable_coil;u32 C;
	C_value=*PLC_Addr%0x100;                //����8λֵ
	PLC_Addr++;
	C_value+=(*PLC_Addr%0x100)*0x100;       //����8λֵ
	if(C_number>=0xC8)                      //�ж��ǲ���C200���ϵ� �Ĵ���
	{  
		PLC_Addr++;
		C_value+=(*PLC_Addr%0x100)*0x10000;  //����8λֵ
		PLC_Addr++;
		C_value+=(*PLC_Addr%0x100)*0x1000000;//����8λֵ
		C=0x20001C00+(C_number-0xC8)*4; 
		temp_bit=1<<(C_number%0x10);
		if(PLC_RAM32(C)<C_value)             //��C��ǰֵ��Ŀ��ֵ���бȽ�
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);//�Ƚ�enable coil 
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			PLC_RAM32(C)+=1;
		}
		if(PLC_RAM32(C)<C_value)              //�Ƚ����ֵ
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
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);  //�Ƚ�enable coil 
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			*p_data+=1;
		}
		if(*p_data<C_value)                   //�Ƚ����ֵ
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
	}
	OUT(0X2700+(u8)C_number);
}

static void enable_C_D(void)	    //�üĴ���D���и�ֵ
{
	static u16 *p_data;
	u16 temp_bit,*p_C_enable_coil;u32 C;
	C_value=PLC_16BIT[0x1000+C_value];
	if(C_number>=0xC8)               //�ж��ǲ���C200���ϵ� �Ĵ���
	{  		 
		C_value+=PLC_16BIT[0x1000+C_value+1]*0x10000;
		C=0x20001C00+(C_number-0xC8)*4; 
		temp_bit=1<<(C_number%0x10);
		if(PLC_RAM32(C)<C_value)      //��C��ǰֵ��Ŀ��ֵ���бȽ�
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);   //�Ƚ�enable coil
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			PLC_RAM32(C)+=1;
		}
		if(*p_data<C_value)            //�Ƚ����ֵ
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
		PLC_Addr+=2;                  
	}
	else
	{
		p_data=PLC_16BIT+0x0500+C_number;
		temp_bit=1<<(C_number%0x10);
		if(*p_data<C_value)    //��C��ǰֵ��Ŀ��ֵ���бȽ�
		{
			p_C_enable_coil=PLC_16BIT+0x0270+(C_number/0X10);   //�Ƚ�enable coil
			if(!((*p_C_enable_coil&temp_bit)==temp_bit))
			*p_data+=1;
		}
		if(*p_data<=C_value)  //�Ƚ����ֵ
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
	}
	OUT(0X2700+(u8)C_number);
}
 
static void disable_C_K(void)
{	
	u32 C;static u16 *p_data;
	C_value=*PLC_Addr%0x100;           //����8λֵ
	PLC_Addr++;
	C_value+=(*PLC_Addr%0x100)*0x100;  //����8λֵ
	if(C_number>=0xC8)                 //�ж��ǲ���C200���ϵ� �Ĵ���
	{  
		PLC_Addr++;
		C_value=(*PLC_Addr%0x100)*0x10000;   //����8λֵ
		PLC_Addr++;
		C_value+=(*PLC_Addr%0x100)*0x1000000;//����8λֵ
		C=0x20001C00+(C_number-0xC8)*4; 
		if(PLC_RAM32(C)<C_value)              //��C��ǰֵ��Ŀ��ֵ���бȽ�
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);        
	}
	else
	{
		p_data=PLC_16BIT+0x0500+C_number;
		if(*p_data<C_value)                 //�Ƚ����ֵ
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);       
	}
	OUT(0X2700+(u8)C_number);
}

static void disable_C_D(void)	     //�رռ�����C
{ 
	u32 C;static u16 *p_data;  
	if(C_number>=0xC8)               //�ж��ǲ���C200���ϵ� �Ĵ���
	{  
		C_value=PLC_16BIT[0x1000+C_value];
		C_value+=PLC_16BIT[0x1000+C_value+1]*0x10000;
		C=0x20001C00+(C_number-0xC8)*4; 
		if(PLC_RAM32(C)<C_value)      //�Ƚ����ֵ
		PLC_BIT_OFF(0x0E00+C_number);		 
		else
		PLC_BIT_ON(0x0E00+C_number);
		PLC_Addr+=2;                  
	}
	else
	{
		C_value=PLC_16BIT[0x1000+C_value];
		p_data=PLC_16BIT+0x0500+C_number;
		if(*p_data<C_value)            //�Ƚ����ֵ
		PLC_BIT_OFF(0x0E00+C_number);
		else
		PLC_BIT_ON(0x0E00+C_number);
	}	
	OUT(0X2700+(u8)C_number);
}

static void C_given_value_K(void)	      //��������K���趨ֵ
{
	if((PLC_ACC_BIT&0X01)==0X01)        //
	enable_C_K();				                  //����������
	else
	disable_C_K(); 
}

static void C_given_value_D(void)	      //��������D���趨ֵ
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
	C_number=*PLC_Addr;       //�������������ĺ�������
	PLC_Addr++;				        //��һ������ȡ��K��ֵ����D��ֵ
	switch(*PLC_Addr/0x100) 
	{
		case 0x80: C_given_value_K();break;  //����K��ֵ����
		case 0x86: C_given_value_D();break;  //����D��ֵ����
	}	
}

static void FNC_AppInstruct(void) 
 { 
	 switch(*PLC_Addr) 
	{
		case 0x0002: PLC_Addr++,expand_OUT();              break;  //M1535���ϵ�ָ��
		case 0x0003: PLC_Addr++,expand_SET();              break;  //M1535���ϵ�ָ��
		case 0x0004: PLC_Addr++,expand_RST();              break;  //M1535���ϵ�ָ��
		
		case 0x0005: PLC_Addr++,expand_OUT();              break;  //
		case 0x0006: PLC_Addr++,expand_SET();              break;  //
		case 0x0007: PLC_Addr++,expand_RST();              break;  //
		case 0x0008: PLC_Addr++,LPS();                     break;  //
		case 0x0009: PLC_Addr++,LPF();                     break;  //
		case 0x000C: PLC_Addr++,RST_T_C();                 break;  //ִ��RST C&T
		case 0x000D: PLC_Addr++,RST_D();                   break;  //ִ��D�Ĵ�����λ
		
		case 0x0010: PLC_Addr++,CJ();                      break;  //CJ  
		case 0x1010: PLC_Addr++,CJP();                     break;  //CJP  
		case 0x0012: PLC_Addr++,CALL();                    break;  //CALL
		case 0x1012: PLC_Addr++,CALLP();                   break;  //CALLP
		case 0x0014: PLC_Addr++,SRET();                    break;  //SRET			
		case 0x001C: PLC_Addr=PLC_Addr;                    break;  //FEND
		
		
	//	case 0X0020: PLC_Addr++,FOR();                     break;  //СС�� 20160929������FORѭ��
	//  case 0X0022: PLC_Addr++,FOR_NEXT();                break;  //СС�� 20160929������FOR_NEST ѭ������
		case 0X0024: PLC_Addr++,CMP();                     break;  //16λ�Ƚϴ���ָ��
		case 0X1024: PLC_Addr++,CMPP();                    break;  //16λ�����رȽϴ���ָ��
		case 0X0025: PLC_Addr++,DCMP();                    break;  //32λ�Ƚϴ���ָ��
		case 0X1025: PLC_Addr++,DCMPP();                   break;  //32λ�����رȽϴ���ָ��
		case 0X0026: PLC_Addr++,ZCP();                     break;  //16λ����ֵ�Ƚϴ���ָ��
		case 0X0027: PLC_Addr++,DZCP();                    break;  //32λ����ֵ�Ƚϴ���ָ��
		case 0x0028: PLC_Addr++,MOV();		                 break;  //ִ��16bit����ָ��
		case 0X0029: PLC_Addr++,DMOV();                    break;  //DMOV 
		case 0X002A: PLC_Addr++,SMOV();                    break;  //SMOV 	
		case 0X002C: PLC_Addr++,CML();                     break;  //CMLȡ��ָ��
		case 0X002D: PLC_Addr++,DCML();                    break;  //DCMLȡ��ָ��
		case 0X002E: PLC_Addr++,BMOV();                    break;  //��������
		case 0X0030: PLC_Addr++,FMOV();                    break;  //��㴫��
		case 0X0031: PLC_Addr++,DFMOV();                   break;  //32λ��㴫��
		case 0X0032: PLC_Addr++,XCH();                     break;  //��������
		case 0X0033: PLC_Addr++,DXCH();                    break;  //32λ��������
		case 0X0034: PLC_Addr++,BCD();                     break;  //������ת��BCD
		case 0X0035: PLC_Addr++,DBCD();                    break;  //������ת��DBCD
		case 0X0036: PLC_Addr++,BIN();                     break;  //������ת��BIN
		case 0X0037: PLC_Addr++,DBIN();                    break;  //������ת��DBIN
		
		case 0X0038: PLC_Addr++,ADD();					           break;  //�ӷ�ָ��
		case 0x0039: PLC_Addr++,DADD();                    break;  //DADD�ӷ�����			
		case 0X003A: PLC_Addr++,SUB();					           break;  //����ָ��
		case 0x003B: PLC_Addr++,DSUB();                    break;  //DSUB��������			
		case 0x003C: PLC_Addr++,MUL();                     break;  //MUL �˷�ָ��
		case 0x003D: PLC_Addr++,DMUL();                    break;  //DMUL�˷�����			
		case 0x003E: PLC_Addr++,DIV();                     break;  //DIV �˷�ָ��
		case 0x003F: PLC_Addr++,DDIV();                    break;  //DDIV��������			
		case 0x0040: PLC_Addr++,INC();                     break;  //16λ�߼������1ָ��
		case 0x1040: PLC_Addr++,INCP();                   break;  //16λ�������߼������1ָ��
		case 0x0041: PLC_Addr++,DINC();                    break;  //32λ�߼������1ָ��
		case 0x1041: PLC_Addr++,DINC_P();                  break;  //32λ�������߼������1ָ��
		case 0x0042: PLC_Addr++,DEC();                     break;  //16λ�߼������1ָ��
		case 0x1042: PLC_Addr++,DECP();                   break;  //16λ�������߼������1ָ��
		case 0x0043: PLC_Addr++,DDEC();                    break;  //32λ�߼������1ָ��
		case 0x0044: PLC_Addr++,WAND();	                   break;  //�߼��������߼�
		case 0x0045: PLC_Addr++,DWAND();	                 break;  //32λ�߼��������߼�
		case 0x0046: PLC_Addr++,WOR();                     break;  //�߼�������߼�
		case 0x0047: PLC_Addr++,DWOR();                    break;  //32λ�߼�������߼�
		case 0x0048: PLC_Addr++,WXOR();                    break;  //�߼���������߼�
		case 0x0049: PLC_Addr++,DWXOR();                   break;  //32λ�߼���������߼�
		case 0x004A: PLC_Addr++,NEG();                     break;  //�߼�����ȡ����
		case 0x004B: PLC_Addr++,DNEG();                    break;  //32λ�߼�����ȡ����			
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
		case 0x0062: PLC_Addr++,DECO();                    break;  //����ת����    
		case 0x006A: PLC_Addr++,MEAN();                    break;	 //MEAN����ƽ��ֵָ��		
		case 0x0070: PLC_Addr++,SQR();	                   break;  //SQR16λ��������			
		case 0x0071: PLC_Addr++,DSQR();	                   break;  //SQR32λ��������
		case 0x0072: PLC_Addr++,FLT();	                   break;  //16λ����ת����
		case 0x0073: PLC_Addr++,DFLT();	                   break;  //32λ����ת����	
		case 0x0076: PLC_Addr++,REFF();	                   break;  //REFF	
		case 0x0078: PLC_Addr++,MTR();	                   break;  //MTR
    case 0x007A: PLC_Addr++,HSCS();		                 break;  //���ټ�����λ  20160709
		
// 	case 0x0084: PLC_Addr++,PWM();                     break;  //PWM���
		case 0x0082: PLC_Addr++,PLSY();                    break;  //�����������
		case 0x0094: PLC_Addr++,ALT();	                   break;  //ALT
//	case 0x00B4: PLC_Addr++,ASCI();	                   break;  //ASCI
		case 0x00C0: PLC_Addr++,PID();	                   break;  //PID
		case 0x00ED: PLC_Addr++,ECMP();	                   break;  //ECMP
		case 0x00EE: PLC_Addr++,EZCP();	                   break;  //EZCP
		
    case 0x00F1: PLC_Addr++,DEMOV();                   break;  //
// 		case 0x00FD: PLC_Addr++,DEBCD();	                 break;  //DEBCD


		case 0x0101: PLC_Addr++,DEADD();                   break;  //����ӷ�����
		case 0x0103: PLC_Addr++,DESUB();	                 break;  //�����������
		case 0x0107: PLC_Addr++,DEDIV();	                 break;  //����˷�����
		case 0x0105: PLC_Addr++,DEMUL();                   break;  //�����������
		case 0x010F: PLC_Addr++,DESQR();                   break;  //DESQR���㿪��
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
		
	
		case 0x01C2: PLC_Addr++,expand_LD();               break;  //M1535���ϵ�ָ��
		case 0x01C3: PLC_Addr++,expand_LDI();              break;  //
		case 0x01C4: PLC_Addr++,expand_AND();              break;  //
		case 0x01C5: PLC_Addr++,expand_ANI();              break;  //
		case 0x01C6: PLC_Addr++,expand_OR();               break;  //
		case 0x01C7: PLC_Addr++,expand_ORI();              break;  //
		
		case 0x01CA: PLC_Addr++,LDP();			               break;  //�����Ӵ������
		case 0x01CB: PLC_Addr++,LDF();			               break;  //�����Ӵ������
		case 0x01CC: PLC_Addr++,ANDP();			               break;  //�����Ӵ������
		case 0x01CD: PLC_Addr++,ANDF();			               break;  //�����Ӵ������
		case 0x01CE: PLC_Addr++,ORP();			               break;  //�����Ӵ������
		case 0x01CF: PLC_Addr++,ORF();			               break;  //�����Ӵ������
		
		
		case 0X01D0: PLC_Addr++,amount();                  break;  //LD 16λ���ڱȽ�
		case 0X01D1: PLC_Addr++,Damount();                 break;  //LD 32λ���ڱȽ�
		case 0X01D2: PLC_Addr++,big();                     break;  //LD 16λ���ڱȽ�
		case 0X01D3: PLC_Addr++,Dbig();                    break;  //LD 32λ���ڱȽ�
		case 0X01D4: PLC_Addr++,less();                    break;  //LD 16λС�ڱȽ�
		case 0X01D5: PLC_Addr++,Dless();                   break;  //LD 32λС�ڱȽ�
		case 0X01D8: PLC_Addr++,no_amount();	             break;  //LD 16λ�����ڱȽ�ָ��
		case 0X01D9: PLC_Addr++,Dno_amount();	             break;  //LD 32λ�����ڱȽ�ָ��
		case 0X01DA: PLC_Addr++,less_amount();             break;  //LD 16λС�ڵ��ڱȽ�
		case 0X01DB: PLC_Addr++,Dless_amount();            break;  //LD 32λС�ڵ��ڱȽ�
		case 0X01DC: PLC_Addr++,big_amount();              break;  //LD 16λ���ڵ��ڱȽ�
		case 0X01DD: PLC_Addr++,Dbig_amount();             break;  //LD 32λ���ڵ��ڱȽ�
		
		case 0X01E0: PLC_Addr++,amount_and();              break;  //LD AND 16λ���ڱȽ�
		case 0X01E1: PLC_Addr++,Damount_and();             break;  //LD AND 32λ���ڱȽ�
		case 0X01E2: PLC_Addr++,big_and();                 break;  //LD AND 16λ���ڱȽ�
		case 0X01E3: PLC_Addr++,Dbig_and();                break;  //LD AND 32λ���ڱȽ�
		case 0X01E4: PLC_Addr++,less_and();                break;  //LD AND 16λС�ڱȽ�
		case 0X01E5: PLC_Addr++,Dless_and();               break;  //LD AND 32λС�ڱȽ�
		case 0X01E8: PLC_Addr++,no_amount_and(); 	         break;  //LD 16λ�����ڱȽ�ָ��
		case 0X01E9: PLC_Addr++,Dno_amount_and(); 	       break;  //LD 32λ�����ڱȽ�ָ��
		case 0X01EA: PLC_Addr++,less_amount_and();         break;  //LD AND 16λС�ڵ��ڱȽ�
		case 0X01EB: PLC_Addr++,Dless_amount_and();        break;  //LD AND 32λС�ڵ��ڱȽ�
		case 0X01EC: PLC_Addr++,big_amount_and();          break;  //LD AND 16λ���ڵ��ڱȽ�
		case 0X01ED: PLC_Addr++,Dbig_amount_and();         break;  //LD AND 32λ���ڵ��ڱȽ�
		
		case 0X01F0: PLC_Addr++,amount_OR();               break;  //LD OR 16λ���ڱȽ�
		case 0X01F1: PLC_Addr++,Damount_OR();              break;  //LD OR 32λ���ڱȽ�
		case 0X01F2: PLC_Addr++,big_OR();                  break;  //LD OR 16λ���ڱȽ�
		case 0X01F3: PLC_Addr++,Dbig_OR();                 break;  //LD OR 32λ���ڱȽ�
		case 0X01F4: PLC_Addr++,less_OR();                 break;  //LD OR 16λС�ڱȽ�
		case 0X01F5: PLC_Addr++,Dless_OR();                break;  //LD OR 32λС�ڱȽ�
		case 0X01F8: PLC_Addr++,no_amount_OR(); 	         break;  //LD 16λ�����ڱȽ�ָ��
		case 0X01F9: PLC_Addr++,Dno_amount_OR(); 	         break;  //LD 32λ�����ڱȽ�ָ��
		case 0X01FA: PLC_Addr++,less_amount_OR();          break;  //LD OR 16λС�ڵ��ڱȽ�
		case 0X01FB: PLC_Addr++,Dless_amount_OR();         break;  //LD OR 32λС�ڵ��ڱȽ�
		case 0X01FC: PLC_Addr++,big_amount_OR();           break;  //LD OR 16λ���ڵ��ڱȽ�
		case 0X01FD: PLC_Addr++,Dbig_amount_OR();          break;  //LD OR 32λ���ڵ��ڱȽ�					
		
		case 0x000F: PLC_Addr=PLC_Addr;                    break;  //�������ENDָ����ʹ�������
		case 0XF7FF: PLC_Addr++,RET();                     break;  //RET
		
		default:PLC_PROG_ERROR(M8065,02); PLC_Addr++;      break;  //������֧�ֵ�����
	}
}
 
void find_p(void)//���� P ���ڵĵ�ַ
{  
	u16 temp;
	PLC_Addr=PLC_START_Address;
	for(temp=0;temp<15999;temp++)//�ܹ�16000��
	{ 
		if((*PLC_Addr&0xFF00)==0xB000)
		PLC_P_Addr[*PLC_Addr%0x100]=PLC_Addr;
		PLC_Addr++;
	}
}

void RST_Y(void)
{PLC_16BIT[80]=PLC_16BIT[81]=0;}

u16 find_toend(void)//���� P ���ڵĵ�ַ
{  
	u16 temp;
	PLC_Addr=PLC_START_Address-1;
	temp=0;	
	do{PLC_Addr++; temp++;}
	while((!(*PLC_Addr==0x000f))&&(temp<15998)); 	 
	return temp; 
}

// СС�ɣ�20160929�Ż�
void PLC_ProInstructParse(void)
{ 
	static u8  puls,run_flag; 
  if(PLC_RUN)    //�Ƿ���Ҫ���г���
  {
    if(run_flag == 1)
	  {
	     run_flag = 0;
		   PLC_8BIT(0X01E0)=0x09;		 
	  }
   if(PLC_8BIT(0X01E0)==0x09)			    //�Ƿ���Ҫ���г���
   {
		PLC_BIT_ON(M8000);	              //����ǿ��M80000ΪON
		PLC_BIT_OFF(M8001);	              //����ǿ��M80001Ϊoff
		run_Lamp;                         //�������е�
		if(edit_prog==0x00)		            //�ж��Ƿ���ڳ���༭������༭һ�γ����������¼���P���ڵ�ַ
		{ 
			find_p();
			edit_prog=1;
			if(find_toend()>15998)
			{  
				PLC_8BIT(0X01E0)=0x09;
				goto all_end;  
			}
		}
		
		if(puls==0x00)		                  //��ʼ�������õ�8002 8003
		{
			PLC_BIT_ON(M8002);
			PLC_BIT_OFF(M8003);
		}
		
		PLC_Addr=PLC_START_Address;        //PLC����ʼ��ַ
		if(Write_Pro_flag == 0)
		{
		   PLC_IO_Refresh();               //ˢ��Y���  
		} 
		
		do
		{
			switch(*PLC_Addr/0x100)          // ȡ��8λ������
			{ 
				case 0x06: operation_T(),PLC_Addr++;                    break;  //operation all timer
				case 0x0E: operation_C(),PLC_Addr++;                    break;  //
				
				/* ����SλԪ�����еĺ��� */
				case 0x20: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x30: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x40: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x50: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x60: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x70: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  // 
				/* ����SλԪ�����еĺ��� */
				case 0x21: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x31: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x41: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x51: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x61: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x71: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* ����SλԪ�����еĺ��� */
				case 0x22: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x32: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x42: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x52: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x62: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x72: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* ����SλԪ�����еĺ��� */
				case 0x23: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x33: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x43: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x53: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x63: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x73: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* ����XλԪ�����еĺ��� */
				case 0x24: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x34: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x44: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x54: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x64: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x74: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				/* ����YλԪ�����еĺ��� */
				case 0x25: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x35: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x45: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x55: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x65: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x75: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC5: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XD5: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XE5: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����TλԪ�����еĺ��� */
				case 0x26: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x36: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x46: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x56: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x66: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x76: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC6: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����TλԪ�����еĺ��� */
				case 0x27: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x37: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x47: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x57: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x67: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x77: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC7: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����M0_255λԪ�����еĺ��� */
				case 0x28: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x38: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x48: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x58: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x68: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x78: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC8: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XD8: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XE8: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //	
				/* ����M256_511λԪ�����еĺ��� */
				case 0x29: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x39: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x49: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x59: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x69: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x79: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XC9: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XD9: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XE9: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����M512_767λԪ�����еĺ��� */
				case 0x2A: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3A: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4A: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5A: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6A: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7A: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCA: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDA: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEA: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����M768_1023λԪ�����еĺ��� */
				case 0x2B: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3B: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4B: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5B: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6B: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7B: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCB: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDB: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEB: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����M1024_1279λԪ�����еĺ��� */
				case 0x2C: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3C: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4C: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5C: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6C: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7C: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCC: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDC: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XEC: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����M1280_1535λԪ�����еĺ��� */
				case 0x2D: LD(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x3D: LDI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0x4D: AND(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x5D: ANI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;
				case 0x6D: OR(0X0FFF&*PLC_Addr),PLC_Addr++;             break;  //
				case 0x7D: ORI(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0XCD: OUT(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				case 0XDD: BIT_SET(0X0FFF&*PLC_Addr),PLC_Addr++;	      break;  //
				case 0XED: RST(0X0FFF&*PLC_Addr),PLC_Addr++;			      break;  //
				/* ����C0-C255λԪ�����еĺ��� */
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
				/**********************STL����ģʽ***************************/
				case 0xF0: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //S
				case 0xF1: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0xF2: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				case 0xF3: STL(0X0FFF&*PLC_Addr),PLC_Addr++;            break;  //
				
				////////////////////////////////////////////////////////
        //�����߼�ָ�������ANB��ORB��MPP��MRD��MPS��INV ��
				case 0XFF: 
				{
					other_function(*PLC_Addr);
				  PLC_Addr++;	      
				  break;   //MPP,MPS
				}
				case 0xB0: //ָ��P��ʶ
				{
					PLC_Addr++;                                  
				  break;  
				}
				case 0x00: //����0X001CΪFEND,0X000FΪENDָ��   
        		{
					if(((*PLC_Addr%0x100)==0x1C)||((*PLC_Addr%0x100)==0x0F))
						goto all_end;
				}
				
				//////////////////////////////////////////////////////////////
				// СС�ɣ�20160929ע�ͣ�Ӧ��ָ��				
				//////////////////////////////////////////////////////////////					
				default: //������֧�ֵ����� ,�˴���Ҫִ������Ϊ16bit��ָ��
				{
					FNC_AppInstruct();                           
					break; 
				}
			}
		}while(1);
all_end: 
			D8010=D8011=D8012=PLC_RUN_TIME;       //����ɨ��ʱ��
			PLC_RUN_TIME=0;                       //���ɨ��ʱ��	
			puls=0x01;		 
			PLC_BIT_OFF(M8002),PLC_BIT_ON(M8003); //��ʼ�������õ�8002 8003
	 }
	else
	{ 
		PLC_BIT_OFF(M8000);	                    //û������ǿ��M80000ΪOFF
		PLC_BIT_ON(M8001);	                    //û������ǿ��M80001Ϊon
		D8012=0; 
		edit_prog=0;	                          //���ʱҪ�õ�
		puls=0; 		                            //��ʼ�������õ�8002 8003
		Stop_Lamp;                              //�ر����е�,���������״̬�л���ֹͣ״̬����Ҫ���Y���
		if(Write_Pro_flag == 0)
		{
		   RST_Y(); 
		   PLC_IO_Refresh();                   //ˢ��Y���  
		   RST_T_D_C_M_data();
		}  
		PLC_STL_CMD = PLC_STL_Status = 0;	     //�ϴγ����еĲ���	
	}
}
else		 
{

   	RST_Y();   												 //���������״̬�л���ֹͣ״̬����Ҫ���Y���                                               
		PLC_BIT_OFF(M8000);	               //û������ǿ��M80000ΪOFF
		PLC_BIT_ON(M8001);	               //û������ǿ��M80001Ϊon
		D8012=0; 
		edit_prog=0;	                     //���ʱҪ�õ�
		puls=0; 													 //��ʼ�������õ�8002 8003		                                          
		Stop_Lamp;                         //�ر����е�
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
	PLC_16BIT[0X701]=0X000;		           //���ð汾��
} 


