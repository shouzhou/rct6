#include "bsp.h"

uint16_t g_timerun=0;
//uint16_t g_Nh3LowEnterCount=0; //��Ũ�Ƚ������ 
uint16_t g_AutoVEntercount=0; //���ģʽ���𵴽׶μ�������
uint8_t g_o3SwitchIndex=1;     //�л����
//uint16_t g_V=1;
float g_AutoVtime =10.0;     //ȫ������ʱ��
float g_AutoOsctime = 10.0 ; //�𵴼���ʱ��
float g_V=0.0;    //ȫ�����ֵ��
extern DHT11_T  g_DHT11;

float FunO3CalParameterKV(float  _allV)
{
    if(_allV>=KVLEVEL1)
        kv = KVL1;
    else if(_allV>=KVLEVEL2)
        kv = KVL2;
    else if(_allV>=KVLEVEL3)
        kv = KVL3;
    else if(_allV>=KVLEVEL4)
        kv = KVL4;
    else if(_allV>=KVLEVEL5)
        kv = KVL5;
    else if(_allV>=KVLEVEL6)
        kv = KVL6;
    else if(_allV>=KVLEVEL7)
        kv = KVL7;
    else if(_allV>=KVLEVEL8)
        kv = KVL8;
    else 
        kv = KVOTHER;
    
}
float FunO3CalParameter(void)
{
    float kh = 1.0;
    float kt = 1.0;
    ////ʪ�Ȳ�������
    if(g_DHT11.Hum>=HLEVEL1)
    {
        kh = KHL1;
    }
    else if(g_DHT11.Hum>=HLEVEL2)
    {
        kh = KHL2;
    }
    else if(g_DHT11.Hum>=HLEVEL3)
    {
        kh = KHL3;
    }
    else if(g_DHT11.Hum>=HLEVEL4)
    {
        kh = KHL4;
    }
    else if(g_DHT11.Hum>=HLEVEL5)
    {
        kh = KHL5;
    }
    else if(g_DHT11.Hum>=HLEVEL6)
    {
        kh = KHL6;
    }
    else if(g_DHT11.Hum>=HLEVEL7)
    {
        kh = KHL7;
    }
    else if(g_DHT11.Hum>=HLEVEL8)
    {
        kh = KHL8;
    }
    else
    {
        kh = KHOTHER;
    }
    
    ////�¶Ȳ��� ����
    if(g_DHT11.Temp>=TLEVEL1)
    {
        kt = KTL1;
    }
    else if(g_DHT11.Temp>=TLEVEL2)
    {
        kt = KTL2;
    }
    else if(g_DHT11.Temp>=TLEVEL3)
    {
        kt = KTL3;
    }
    else if(g_DHT11.Temp>=TLEVEL4)
    {
        kt = KTL4;
    }
    else if(g_DHT11.Temp>=TLEVEL5)//40
    {
        kt = KTL5;
    }
    else if(g_DHT11.Temp>=TLEVEL6)//30
    {
        kt = KTL6;
    }
    else if(g_DHT11.Temp>=TLEVEL7)//25
    {
        kt = KTL7;
    }
    else if(g_DHT11.Temp>=TLEVEL8)//20
    {
        kt = KTL8;
    }
    else if(g_DHT11.Temp>=TLEVEL9)//10
    {
        kt = KTL9;
    }
    else if(g_DHT11.Temp>=TLEVEL10)//5
    {
        kt = KTL10;
    }
    else
    {
        kt = KTOTHER;
    }
    
    if(g_chemical)
    {
        
    }
    else
    {
        
    }
    if(g_plant)
    {
        
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

void FunSwitch(void)
{
    #ifdef USEDEBUG
    if(g_status != 26)
    printf("current status = %d\r\n",g_status);
    
    
    #endif
    if((g_status>10)&&(g_status<STATUS_AUTOV_WAIT))
        g_sysruntime ++;
    
        switch(g_status)
        {
                case STATUS_ALL_STOP :
                  //ȷ������ģ�鶼�ǹرյ�
                FunO3Control(0,1);
                FunFanClose();
                
                   
                  break;
//                case STATUS_NETINIEND: //�����ʼ����ɣ��ȴ�����ʵ������
//                    FunO3Control(0,1);
//                    FunFanClose();
//                    // ����ѡ������Ӧ�Ĳ�ͬģʽ
//                 if(g_funcKey == STARTAUTOV) //�Զ����ģʽ
//                   {
//                      g_status = STATUS_AUTOV_START; //�����Զ����ģʽ
//                   }
//                    break;
                   
                case STATUS_AUTOV_START:
                    FunO3Control(0,1);//1-4��
                    FunFanOpen(); //���ȿ�
                    g_status = STATUS_CHECK_F1_1;
                    g_touch = 11;  //������ʾ��  ��ʼ����
                    TIM_Cmd(TIM6,DISABLE ); //�رմ����Ͱ�������
                    break;
                case STATUS_CHECK_F1_1://2���� �̶�ʱ�� ���޸���
                    g_timerun++;
                    if(g_timerun>120)
                    {
                        g_status = STATUS_CHECK_F1_2;
                        g_timerun = 0;
                        FunFanClose();
                    }
                    break;
                    case STATUS_CHECK_F1_2://�������������ʱ��
                    g_timerun++;
                    if(g_timerun>CHECKSTARTTIME) //��Сʱ �Լ��޸�ʱ��  
                    {
                        g_status = STATUS_CHECK_F2;
                        g_timerun = 0;
                        FunFanOpen();
                    }
                    break;
                case STATUS_CHECK_F2:
                    DHT11_ReadData(&g_DHT11);
                    if(g_DHT11.Hum>=CHECKL1HUM) //>=85%
                    {
                        FunO3Control(4,1);//1-4��
                       // FunFanClose(); //���ȹ�
                        g_status = STATUS_CHECK_F3_L1; 
                    }
                    else if(g_DHT11.Hum>CHECKL2HUM)//>=65% <=85%
                    {
                        FunO3Control(4,1);//1-4��
                       // FunFanClose(); //���ȹ�
                        g_status = STATUS_CHECK_F3_L2; 
                    }
                    else if(g_DHT11.Hum>CHECKL3HUM)//>=55% <= 65%
                    {
                        FunO3Control(4,1);//1-4��
                       // FunFanClose(); //���ȹ�
                        g_status = STATUS_CHECK_F3_L3; 
                    }
                    else if(g_DHT11.Hum>=CHECKL4HUM) //>=50% <= 55%
                    {
                        FunO3Control(4,1);//1-4��
                       // FunFanClose(); //���ȹ�
                        g_status = STATUS_CHECK_F3_L4;
                    }
                    else // <50%  -���� ֹͣ����
                    {
                        g_status = STATUS_CHECK_F3_L5;
                    }
                    break;
                case STATUS_CHECK_F3_L1://���ʪ����ʱ
                    g_timerun++;
                    if(g_timerun>CHECKL1TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                
                    break;
                case STATUS_CHECK_F3_L2://�θ�ʪ����ʱ
                    g_timerun++;
                    if(g_timerun>CHECKL2TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F3_L3://����ʪ����ʱ
                    g_timerun++;
                    if(g_timerun>CHECKL3TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F3_L4://����ʪ����ʱ
                    g_timerun++;
                    if(g_timerun>CHECKL4TIME)
                    {
                        g_status = STATUS_CHECK_F4;
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F3_L5://ͣ���ȴ�ʪ�ȱ��������
                    g_timerun++;
                    if(g_timerun>5)
                    {
                         DHT11_ReadData(&g_DHT11);
                         if(g_DHT11.Hum>=CHECKL4HUM) //>=50%
                        {
                            FunO3Control(4,1);//1-4��
                           // FunFanClose(); //���ȹ�
                            g_status = STATUS_CHECK_F3_L4;
                        }
                        g_timerun = 0;
                    }
                    break;
                case STATUS_CHECK_F4://
                     g_status = STATUS_AUTOV_RUN_F1;
//                    g_timerun++;
//                    tim_print_result();
//                    if(g_timerun>=5)
//                    {
//                        g_timerun = 0;
//                        if((g_checkfreok[0]>=3)&&(g_checkfreok[1]>=3)&&(g_checkfreok[2]>=3)&&(g_checkfreok[3]>=3))
//                        {
//                            g_status = STATUS_AUTOV_RUN_F1;
//                            g_checkfreok[0] =0;
//                            g_checkfreok[1] =0;
//                            g_checkfreok[2] =0;
//                            g_checkfreok[3] =0;
//                            TIM_Cmd(TIM3,DISABLE ); //�رղ�Ƶ����
//                            bsp_ScreenUPdateSYSSTATUS(g_status);
//                            bsp_DelayMS(100);
//                            bsp_ScreenUPdateSYSSTATUS(g_status);
//                        }
//                        else
//                        {
//                             g_status = STATUS_CHECK_FAIL;
//                        }
//                        
//                    }
                    
                    break;
                case STATUS_CHECK_FAIL :
                    
                    break;
 //--------------------------�Զ����ģʽ ״̬��ѯ------------------begin----------------------------//  
                case STATUS_AUTOV_RUN_F1://2����
                    g_status = STATUS_AUTOV_RUN_F2;  //����������  STATUS_AUTOV_RUN_F2  ����� STATUS_AUTOV_RUN_F1
                    TIM_Cmd(TIM3,DISABLE ); //�رղ�Ƶ����
                    
//                   g_timerun++;
//                   if(g_timerun>120)
//                   {
//                      g_timerun =0;
//                      g_status = STATUS_AUTOV_RUN_F2;
//                   }
                    break;
                case STATUS_AUTOV_RUN_F2: //��һ�μ���
                    kp = FunO3CalParameter();  //0.4
                    g_V = g_lenth*g_width*g_height/1000; //200
                    kv= FunO3CalParameterKV(g_V);  //kv  1.4
                    g_AutoVtime = kv*g_lenth*g_width*g_height/1000.0-60*kp;  //  �������ʱ������  ��һ��g_AutoVtime /24
                    g_status = STATUS_AUTOV_RUN_F3;
                    break;
                case STATUS_AUTOV_RUN_F3: //���ݼ���ʱ�䣬�ж���һ����ʲô
                   
                    if(g_AutoVtime<AUTOVCALTIME)
                    {
                       g_status = STATUS_AUTOV_RUN_ON; //������ģʽ
                       FunO3Control(2,g_o3SwitchIndex);//��2��
                        g_touch = 12;
                        g_AutoOsctime = kc*g_lenth*g_width*g_height/1000.0;   //����ʱ�� KC *gv
                    }
                    else  //ʱ��δ��
                    {
                       g_status = STATUS_AUTOV_RUN_F4; //�ȴ�2����
                    }
                    break;

                case STATUS_AUTOV_RUN_F4: //�ȴ�2���� ���ٴμ���
                    g_timerun++;
                   if(g_timerun>AUTOVCALTIME)
                   {
                      kp = FunO3CalParameter();
                      g_AutoVtime = g_AutoVtime - AUTOVCALTIME*kp; //T2 = T1 -120*KP
                      printf("kp = %f, kv=%f, the time is %f osc time =%f\r\n",kp,kv,g_AutoVtime,kc*g_lenth*g_width*g_height/1000.0);
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_F3; //������ȥ����ʱ��
                   }
                    break;
                case STATUS_AUTOV_RUN_ON://��ģʽ ���׶�
                     kp = FunO3CalParameter();
                     g_AutoOsctime = g_AutoOsctime-kp;
                     printf("OSC TIME = %5.1f\r\n",g_AutoOsctime);
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>AUTOVONTIME)  //�̶�2����
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_OFF; //������ �ؽ׶�
                      FunO3Control(0,g_o3SwitchIndex);
                   }
                   if(g_AutoOsctime<2)
                  // if(g_AutoVEntercount>g_AutoOsctime)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       FunO3Control(0,1);
                       g_status = STATUS_AUTOV_WAIT_D1;
                   }
                    break;
                case STATUS_AUTOV_RUN_OFF://��ģʽ �ؽ׶�
                     kp = FunO3CalParameter();
                     g_AutoOsctime = g_AutoOsctime-kp;
                     printf("OSC TIME = %5.1f\r\n",g_AutoOsctime);
                     g_timerun++;
                     g_AutoVEntercount++;
                   if(g_timerun>koscoff*100000.0/(g_lenth*g_width*g_height/1000.0)) //�𵴹�ʱ��������ɷ���  0.9345*100000/g_V 
                   {
                      g_timerun =0;
                      g_status = STATUS_AUTOV_RUN_ON; //������ ���׶�
                      FunO3Control(2,1);
                      
                   }
                  //  if(g_AutoVEntercount>g_AutoOsctime)
                   if(g_AutoOsctime<2)
                   {
                       g_timerun =0;
                       g_AutoVEntercount=0;
                       g_status = STATUS_AUTOV_WAIT_D1;
                       FunO3Control(0,1);
                   }
                    break;
                 case STATUS_AUTOV_WAIT_D1://�ȴ�����
                     g_timerun++;
                     if(g_timerun>FANOFFTIME)
                   {
                      g_timerun =0;
                      if((g_chemical ==1) &&(g_secondrun==0))//������
                      {
                          g_secondrun =1;
                           g_status = STATUS_AUTOV_START; //���¿�ʼһ��
                      }
                      else
                      {
                        g_status = STATUS_AUTOV_WAIT; //�������״̬
                        FunFanClose();
                      }
                     
                      
                   }
                   
                    break;
                  case STATUS_AUTOV_WAIT:
                     g_timerun++;
                     bsp_ScreenUPdateSYSSTATUS(g_status);  

                     if(g_timerun>5)
                     {
                         g_timerun =0;
                         g_status = STATUS_AUTOV_STOP_D1;
                     }
                    break;
                 
                     
                  case STATUS_AUTOV_STOP_D1://������
                     g_timerun++;
                   
                     if(g_timerun %5 ==0)
                     {
                       bsp_PlayMusic();  
                         
                     }
                     
                    break;
                
                 
//---------------------------�Զ����ģʽ ״̬��ѯ------------------end----------------------------//    
                default:
                    break;
            
        }
        g_funcKey=0xff;  //g_func �ָ���ʼֵ
}
//-----------------------FunSwitch--------״̬�л� end-------------------------//


//ϵͳ�Լ캯�� �����ϵͳ�����Ƿ�ok 
void bsp_SysCheck(void)
{
//    uint8_t tempsec;
//    RTC_ReadClock();
//    printf("%4d-%02d-%02d %02d:%02d:%02d\r\n", g_tRTC.Year, g_tRTC.Mon, g_tRTC.Day, 
//                  g_tRTC.Hour, g_tRTC.Min, g_tRTC.Sec);
//    while(1)
//    {
//       	/* ��ʱ�ӣ���������ȫ�ֱ��� g_tRTC */
//        if(tempsec != g_tRTC.Sec)
//        {
//            RTC_ReadClock();
//            tempsec=g_tRTC.Sec;
//            switch(g_status)
//            {   
//                case  STATUS_CHECK_F1:
//                    DHT11_ReadData(&g_DHT11);
//                    if(g_DHT11.Hum>=850) //>=85%
//                    {
//                        
//                    }
//                    else if(g_DHT11.Hum>650)//>=65% <=85%
//                    {
//                        
//                    }
//                    else if(g_DHT11.Hum>550)//>=55% <= 65%
//                    {
//                        
//                    }
//                    break;
//                case  STATUS_CHECK_F2:
//                    break;
//                case  STATUS_CHECK_F3:
//                    break;
//                case  STATUS_CHECK_F4:
//                    break;
//                case  STATUS_CHECK_F5:
//                    break;
//                default:
//                    break;
//                
//            }
//        }
//    }
}

void FunTouch(void)
{
    if(g_touch==1)   //����������ȷʱ
    {
        bsp_SendKey(3); //��ת���������
        bsp_DelayMS(100);
        bsp_SendKey(3);
        bsp_DelayMS(1);
        g_touch = 0xff;
            
    }
    else if(g_touch==2)  //�����������ʱ
    {
        bsp_SendKey(4); //�ص�״̬����
        bsp_DelayMS(5);
        bsp_SendKey(4); //�ص�״̬����
        bsp_DelayMS(5);
        bsp_DisableTouch();//�رմ���
        g_touch = 0xff;
    }
    else if(g_touch==3)  //����--ȷ����ť
    {
        bsp_DisableTouch(); //�رմ���
        bsp_DelayMS(50);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(50);
        bsp_ScreenUpdateNet(g_status,g_WithoutOnenet);
        bsp_DelayMS(10);
        g_touch = 0xff;
    }
    else if(g_touch==4)  //����--��������
    {
        eeResult[7] = 0;  //��λʧ�ܴ���
        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
        nbiot_reset();
        g_touch = 0xff;
    }
    else if(g_touch==5)  //����--�ر�����
    {
        eeResult[7] = 60;  //��λʧ�ܴ���
        ee_WriteBytes(0,&eeResult[7],0x07,0x01); //дʧ�ܴ���
        nbiot_reset();
        g_touch = 0xff;
    }
    else if(g_touch==6) //����-�����������
    {
        bsp_SendKey(0x02);  
        bsp_DelayMS(100); 
        bsp_SendKey(0x02);  
        bsp_DelayMS(100);
        bsp_EnableTouch();
        bsp_DelayMS(100);
        bsp_EnableTouch();
        bsp_DelayMS(10);
        g_touch = 0xff;
 
 
    }
    else if((g_touch==7)||(g_touch ==8)) //�������� ����
    {
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
        bsp_ScreenUPdateChemical(g_chemical);
        bsp_DelayMS(100);
                    
        eeResult[1] = g_chemical;
        eeResult[0] |= 0x10;                       //��־ ��ѧ�����Ѿ�����
        ee_WriteBytes(0,eeResult,0x00,0x01);       //д��־
        ee_WriteBytes(0,&eeResult[1],0x0f,0x01);   //д��ѧ����
        
        g_touch = 0xff;
     }
    
     else if((g_touch==9)||(g_touch ==10)) //ֲ������ ����
    {
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        bsp_ScreenUPdateType(g_plant);
        bsp_DelayMS(100);
        
        eeResult[1] = g_plant;
        eeResult[0] |= 0x08;                       //��־ ֲ�������Ѿ�����
        ee_WriteBytes(0,eeResult,0x00,0x01);       //д��־
        ee_WriteBytes(0,&eeResult[1],0x0e,0x01);   //дֲ������
        
        g_touch = 0xff;
     }
    else if((g_touch == 11)||(g_touch == 12))  //������ʾ��  ��ʼ����
    {
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        g_touch = 0xff;
    }
    else if(g_touch == 13)  //������ʾ��  ��ʼ����
    {
        bsp_ScreenUPdateSYSSTATUS(g_status);
        bsp_DelayMS(100);
        bsp_ScreenUPdateSYSSTATUS(g_status);
        g_touch = 0xff;
    }
    
}


