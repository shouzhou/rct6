#include "bsp.h"
#define LOWMAXTIME      300   //�޸�����꣬�ı��Ũ��ʱ���Ϲص���ʱ��
#define LOWSWITCHTIME   60    //�޸�����꣬�ı��Ũ��ʱÿ�ο��͹ص�ʱ��
#define FANOFFTIME      120   //�޸�����꣬�ı����ӳٹر�ʱ��
#define AUTOVONTIME    120     //�޸�����꣬�ı��Զ����ģʽ���𵴿�ʱ��
#define AUTOVOFFTIME   120     //�޸�����꣬�ı��Զ����ģʽ���𵴹�ʱ��

#define O3LEVEL1        50    //�޸�����꣬�ı����Ũ��ֵ1
#define O3LEVEL2        40    //�޸�����꣬�ı����Ũ��ֵ2
#define O3LEVEL3        10    //�޸�����꣬�ı����Ũ��ֵ3
#define O3LEVEL4        0     //�޸�����꣬�ı����Ũ��ֵ4-�����������
//ʪ�ȷֶ�
#define HLEVEL1    950 
#define HLEVEL2    850 
#define HLEVEL3    800 
#define HLEVEL4    750 
#define HLEVEL5    650 
#define HLEVEL6    600 
#define HLEVEL7    550 
#define HLEVEL8    550 
//�¶ȷֶ�
#define TLEVEL1    600 
#define TLEVEL2    550 
#define TLEVEL3    500 
#define TLEVEL4    450 
#define TLEVEL5    400 
#define TLEVEL6    300 
#define TLEVEL7    250 
#define TLEVEL8    200 
#define TLEVEL9    100 



uint8_t g_status =0;   //ϵͳ���е�����״̬�л�
uint16_t g_timerun=0;
uint16_t g_Nh3LowEnterCount=0; //��Ũ�Ƚ������ 
uint16_t g_AutoVEntercount=0; //���ģʽ���𵴽׶μ�������
uint8_t g_o3SwitchIndex=1;     //�л����
uint16_t g_V=1;
float g_AutoVtime =10.0;
float kv = 1.0;
float kc = 1.0;
extern DHT11_T  g_DHT11;
float FunO3CalParameter(void)
{
    float kh = 1.0;
    float kt = 1.0;
    ////ʪ�Ȳ�������
    if(g_DHT11.Hum>=HLEVEL1)
    {
        kh = 0.3;
    }
    else if(g_DHT11.Hum>=HLEVEL2)
    {
        kh = 0.35;
    }
    else if(g_DHT11.Hum>=HLEVEL3)
    {
        kh = 0.4;
    }
    else if(g_DHT11.Hum>=HLEVEL4)
    {
        kh = 0.5;
    }
    else if(g_DHT11.Hum>=HLEVEL5)
    {
        kh = 0.6;
    }
    else if(g_DHT11.Hum>=HLEVEL6)
    {
        kh = 0.8;
    }
    else if(g_DHT11.Hum>=HLEVEL7)
    {
        kh = 0.9;
    }
    else if(g_DHT11.Hum>=HLEVEL8)
    {
        kh = 1;
    }
    else
    {
        kh = 1;
    }
    
    ////�¶Ȳ��� ����
    if(g_DHT11.Temp>=TLEVEL1)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL2)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL3)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL4)
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL5)//40
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL6)//30
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL7)//25
    {
        kt = 0.8;
    }
    else if(g_DHT11.Temp>=TLEVEL8)//20
    {
        kt = 0.98;
    }
    else if(g_DHT11.Temp>=TLEVEL9)//10
    {
        kt = 1;
    }
    else
    {
        kt = 1;
    }
    return kh*kt;
}
void FunFanOpen(void)
{
    bsp_IOOff(5);
}
void FunFanClose(void)
{
    bsp_IOOn(5);
}
// onnum ���ĸ���
// index  �ӵڼ��������ݶ�һ��Сʱ��ʼ�л�˳��  1-4
// 
void FunO3Control(uint8_t onnum,uint8_t index)
{
    uint8_t offnum = 4-onnum; //�رո���
    uint8_t i,j;
    for(i=index;i<onnum+index;i++) //��������
    {
        if(i>4)
            bsp_IOOff(i-4);
        else
            bsp_IOOff(i);
    }
    for(j=i;j<offnum+i;j++)
    {
        if(j>4)
            bsp_IOOn(j-4);
        else
            bsp_IOOn(j);
    }
   
}
extern uint16_t g_nh3den;
void FunCheckStatus(void)  //�Զ�Ũ��ģʽ�µ�״̬�л�
{
    
        if(g_nh3den >O3LEVEL1) //>50ppm -ȫ��
        {
             g_status = STATUS_AUTODEN_RUN_F1;             
        }
        else if(g_nh3den >O3LEVEL2) //40 -50ppm ������
        {
             g_status = STATUS_AUTODEN_RUN_F2; 
        }
        else if(g_nh3den >O3LEVEL3) //10 -40 ppm ������
        {
             g_status = STATUS_AUTODEN_RUN_F3; 
        }
        else if(g_nh3den >=O3LEVEL4)//1 -10 ppm ѭ������
        {
            g_status = STATUS_AUTODEN_RUN_F4; 
        }
        g_Nh3LowEnterCount =0;
        g_timerun =0;
 }                  
void FunSwitch(void)
{
        switch(g_status)
        {
            case STATUS_ALL_STOP :
              //ȷ������ģ�鶼�ǹرյ�
            FunO3Control(0,g_o3SwitchIndex);
            FunFanClose();
            // ����ѡ������Ӧ�Ĳ�ͬģʽ
               if(g_func == STARTMANUAL) //�ֹ�ģʽ
               {
                  g_status = STATUS_MANUAL_START; //�����ֹ�ģʽ
               }
               else if(g_func == STARTLOOP) //ѭ��ģʽ
               {
                  g_status = STATUS_LOOP_START; //����ѭ��ģʽ
               }
               else if(g_func == STARTAUTODEN) //�Զ�Ũ��ģʽ
               {
                  g_status = STATUS_AUTODEN_START; //�����Զ�Ũ��ģʽ
               }
               else if(g_func == STARTAUTOV) //�Զ����ģʽ
               {
                  g_status = STATUS_AUTOV_START; //�����Զ����ģʽ
               }
               
              break;
//---------------------------�ֹ�ģʽ ״̬��ѯ------------------begin----------------------------//               
            case STATUS_MANUAL_START: //�����ֹ�ģʽ �������̵�����ֻ��һ��
                FunO3Control(4,g_o3SwitchIndex);
                FunFanOpen();
                g_status = STATUS_MANUAL_RUN_D1; //�����ֹ�ģʽ��ʱ�׶�
              
                
                break;
            case STATUS_MANUAL_RUN_D1: 
                g_timerun++;
                if(g_func ==STOPMANUAL) // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                {
                   g_timerun =0;
                   g_status = STATUS_MANUAL_STOP_D1;
                   FunO3Control(0,g_o3SwitchIndex);
                }
               if(g_timerun>g_mannualtime/10*60)
               {
                   g_status = STATUS_MANUAL_STOP_D1; //����ֹͣǰ�ȴ�ģʽ
                   g_timerun =0 ; //��ʱֵ����
                   // �ȴ�ʱ�䵽 �رճ��� ���ַ��ȿ���
                   FunO3Control(0,g_o3SwitchIndex);
               }
                break;
            case STATUS_MANUAL_STOP_D1:
                g_timerun++;
               if(g_timerun>FANOFFTIME) //�ȴ�300S�� ���йر�
               {
                   g_status = STATUS_ALL_STOP; //����ֹͣǰ�ȴ�ģʽ
                   g_timerun =0 ; //��ʱֵ����
                   
               }
                break;
//---------------------------�ֹ�ģʽ ״̬��ѯ------------------end----------------------------// 
               
               

//---------------------------ѭ��ģʽ ״̬��ѯ------------------begin----------------------------//                 

            case  STATUS_LOOP_START :
                FunO3Control(4,g_o3SwitchIndex);
                FunFanOpen();
                g_status = STATUS_LOOP_RUN_F1; //����ѭ��ģʽ
                break;
            case STATUS_LOOP_RUN_F1: //ѭ���е������׶�
                g_timerun++;
               if(g_func ==STOPLOOP) // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                {
                   g_timerun =0;
                   g_status = STATUS_LOOP_STOP_D1;
                   FunO3Control(0,g_o3SwitchIndex);
                }
               if(g_timerun>g_loopruntime /10 * 60) //�ȴ�����S�� ����ѭ��ֹͣ�׶�
               {
                   g_status = STATUS_LOOP_RUN_F2; //����ѭ���е�ֹͣ�׶�
                   g_timerun =0 ; //��ʱֵ����
                   FunO3Control(0,g_o3SwitchIndex);
                   
               }
                break;
            case STATUS_LOOP_RUN_F2: //ѭ���е�ֹͣ�׶�
                g_timerun++;
                if(g_func ==STOPLOOP) // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                {
                   g_timerun =0;
                   g_status = STATUS_LOOP_STOP_D1;
                   FunO3Control(0,g_o3SwitchIndex);
                }
               if(g_timerun>g_loopstoptime /10 * 60) //�ȴ�����S�� ����ѭ��ֹͣ�׶�
               {
                   g_status = STATUS_LOOP_RUN_F1; //����ѭ���еĿ����׶�
                   g_timerun =0 ; //��ʱֵ����
                   FunO3Control(4,g_o3SwitchIndex);
                   
               }
                break;
               
              case STATUS_LOOP_STOP_D1: //��ʱ����ģʽ
               g_timerun++;
               if(g_timerun>FANOFFTIME) //�ȴ�300S�� ���йر�
               {
                   g_status = STATUS_ALL_STOP; //����ֹͣǰ�ȴ�ģʽ
                   g_timerun =0 ; //��ʱֵ����
                   
               }
                break;
            
            
            
//---------------------------ѭ��ģʽ ״̬��ѯ------------------end----------------------------//     

    
               

//---------------------------�Զ�Ũ��ģʽ ״̬��ѯ------------------begin----------------------------// 
               case STATUS_AUTODEN_START: //�Զ�Ũ��ģʽ��ʼ
                FunO3Control(4,g_o3SwitchIndex);
                FunFanOpen();
                FunCheckStatus();

                break;
               case STATUS_AUTODEN_RUN_F1://>50
                  FunO3Control(4,g_o3SwitchIndex);  
                 if(g_nh3den<=O3LEVEL1) FunCheckStatus();
                 if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
               case STATUS_AUTODEN_RUN_F2://>40-50
                 FunO3Control(3,g_o3SwitchIndex); 
                 if((g_nh3den>O3LEVEL1)||(g_nh3den<=O3LEVEL2)) FunCheckStatus();
                 if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
               case STATUS_AUTODEN_RUN_F3://>10-40
                 FunO3Control(2,g_o3SwitchIndex);
                if((g_nh3den>O3LEVEL2)||(g_nh3den<=O3LEVEL3)) FunCheckStatus();
                if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
               case STATUS_AUTODEN_RUN_F4: //��Ũ��ģʽ ��ֻ��һ�� Ȼ����뿪��ѭ������ģʽ
                   FunO3Control(1,g_o3SwitchIndex);
                   g_status = STATUS_AUTODEN_RUN_F4_ON;
                   break;
               case STATUS_AUTODEN_RUN_F4_ON://>=0
                   if((g_nh3den>O3LEVEL4)) FunCheckStatus();
                   g_timerun++;
                   g_Nh3LowEnterCount++;
                  if(g_timerun>LOWSWITCHTIME) //��ʼ�����Ũ��offģʽ
                  {
                    FunO3Control(0,g_o3SwitchIndex);
                    g_timerun =0;
                    g_status = STATUS_AUTODEN_RUN_F4_OFF;
                      
                  }
                  if(g_Nh3LowEnterCount>LOWMAXTIME) //��������ȴ�
                  {
                    g_status =  STATUS_AUTODEN_WAIT_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                  }
                  if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
                  
                case STATUS_AUTODEN_RUN_F4_OFF://>=0
                   if((g_nh3den>O3LEVEL4)) FunCheckStatus();
                   g_timerun++;
                   g_Nh3LowEnterCount++;
                  if(g_timerun>LOWSWITCHTIME) //��ʼ�����Ũ��onģʽ
                  {
                    FunO3Control(1,g_o3SwitchIndex);
                    g_timerun =0;
                    g_status = STATUS_AUTODEN_RUN_F4_ON;
                      
                  }
                  if(g_Nh3LowEnterCount>LOWMAXTIME) //��������ȴ�
                  {
                    g_status =  STATUS_AUTODEN_WAIT_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                  }
                  if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                 break;
                  
                case STATUS_AUTODEN_STOP_D1:
                g_timerun++;
               if(g_timerun>FANOFFTIME) //�ȴ�300S�� ���йر�
               {
                   g_status = STATUS_ALL_STOP; //����ر�ģʽ
                   g_timerun =0 ; //��ʱֵ����
                   FunFanClose(); //�رշ��
               }
                    break;
                case STATUS_AUTODEN_WAIT_D1: //�����ȴ�
                    g_timerun++;
                   if(g_timerun>FANOFFTIME) //�ȴ�300S�� ���йر�
                   {
                       g_status = STATUS_AUTODEN_WAIT; //�������ģʽ
                       g_timerun =0 ; //��ʱֵ����
                       FunFanClose(); //�رշ��
                   }
                    if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                     {
                        g_status =  STATUS_AUTODEN_STOP_D1;
                        g_Nh3LowEnterCount =0;
                        g_timerun =0;
                        FunO3Control(0,g_o3SwitchIndex);
                     }
                    break;
                
                case STATUS_AUTODEN_WAIT : //����ģʽ
                    if(g_nh3den>10)   //Ũ������ʱ��ʼ�л�
                    {
                        FunCheckStatus();
                        FunFanOpen();
                    }
                    if(g_func == STOPAUTODEN)  // �����й����� ����� �رհ�ť ֱ����ת����ʱ����ģʽ
                 {
                    g_status =  STATUS_AUTODEN_STOP_D1;
                    g_Nh3LowEnterCount =0;
                    g_timerun =0;
                    FunO3Control(0,g_o3SwitchIndex);
                 }
                   break;

//---------------------------�Զ�Ũ��ģʽ ״̬��ѯ------------------end----------------------------//   
    

    
               

//---------------------------�Զ����ģʽ ״̬��ѯ------------------begin----------------------------//  
                case STATUS_AUTOV_START:
                    FunO3Control(4,g_o3SwitchIndex);
                    FunFanOpen();
                    g_status = STATUS_AUTOV_RUN_F1;
                    break;
                case STATUS_AUTOV_RUN_F1://2����
                   g_timerun++;
                   if(g_timerun>120)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_F2;
                   }
                    break;
                case STATUS_AUTOV_RUN_F2:
                    g_AutoVtime = kv*g_lenth*g_width*g_height-2*FunO3CalParameter();
                    g_status = STATUS_AUTOV_RUN_F3;
                    break;
                case STATUS_AUTOV_RUN_F3: //ѭ������ʱ��
                    g_AutoVtime = g_AutoVtime - 2*FunO3CalParameter();
                    if(g_AutoVtime<5)
                    {
                       g_status = STATUS_AUTOV_RUN_ON; //������ģʽ
                       FunO3Control(2,g_o3SwitchIndex);//��2��
                    }
                    else
                    {
                       g_status = STATUS_AUTOV_RUN_F4; //�ȴ�2����
                    }
                    break;
                case STATUS_AUTOV_RUN_F4:
                    g_timerun++;
                   if(g_timerun>120)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_F3; //������ȥ����ʱ��
                   }
                    break;
                case STATUS_AUTOV_RUN_ON://��ģʽ ���׶�
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>AUTOVONTIME)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_OFF; //������ �ؽ׶�
                      FunO3Control(0,g_o3SwitchIndex);
                   }
                   if(g_AutoVEntercount>kc*g_lenth*g_width*g_height)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       g_status = STATUS_AUTOV_WAIT_D1;
                   }
                    break;
                case STATUS_AUTOV_RUN_OFF://��ģʽ ���׶�
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>AUTOVOFFTIME)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_ON; //������ ���׶�
                      FunO3Control(2,g_o3SwitchIndex);
                       FunO3Control(0,1);
                   }
                    if(g_AutoVEntercount>kc*g_lenth*g_width*g_height)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       g_status = STATUS_AUTOV_WAIT_D1;
                       FunO3Control(0,1);
                   }
                    break;
                 case STATUS_AUTOV_WAIT_D1://�ȴ�����
                     g_timerun++;
                     if(g_timerun>300)
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_WAIT; //������ ���׶�
                     
                       FunFanClose();
                   }
                   
                    break;
                  case STATUS_AUTOV_WAIT://������
                     g_timerun++;
                                        
                    break;
                
                 
//---------------------------�Զ����ģʽ ״̬��ѯ------------------end----------------------------//    
            default:
                break;
            
        }
        g_func=0xff;  //g_func �ָ���ʼֵ
}
//-----------------------FunSwitch--------״̬�л� end-------------------------//
