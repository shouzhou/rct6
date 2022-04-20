
#ifndef __SET_APP_H
#define __SET_APP_H
#include "stdint.h"

#define NETUPDATETIME        60   //������ʱ���¼��
#define NONETUPDATETIME      10    //������ʱ���¼��
////////�Լ����////////////////////////////

#define CHECKSTARTTIME        120    //��ʼ��ʱ��  ��λS
#define CHECKL1TIME           180    //�Լ�׶����ʪ����ʱʱ��
#define CHECKL2TIME           120    //�Լ�׶εڶ�ʪ����ʱʱ��
#define CHECKL3TIME           30     //�Լ�׶ε������ʪ����ʱʱ��
#define CHECKL4TIME           3      //�Լ�׶ε���ʪ����ʱʱ��


#define CHECKL1HUM           850    //�Լ�׶����ʪ��   10����ϵ
#define CHECKL2HUM           650    //�Լ�׶εڶ�ʪ��
#define CHECKL3HUM           550    //�Լ�׶ε���ʪ��
#define CHECKL4HUM           500    //�Լ�׶ε���ʪ��

#define CHECKFRELOW          8      //�Լ�׶�Ƶ�ʵ�ֵ   ��λKHZ         
#define CHECKFREHIGH         20      //�Լ�׶�Ƶ�ʵ�ֵ


//////���в���
#define FANOFFTIME     120     //�޸�����꣬�ı����ӳٹر�ʱ��  ��λS
#define AUTOVONTIME    120     //�޸�����꣬�ı��Զ����ģʽ���𵴿�ʱ��
#define AUTOVOFFTIME   120     //�޸�����꣬�ı��Զ����ģʽ���𵴹�ʱ��
#define AUTOVCALTIME   120     //�޸�����꣬�ı��Զ����ģʽ��ÿ�μ�������ļ��ʱ��


//ʪ�ȷֶ�---10��
#define HLEVEL1    950 
#define HLEVEL2    850 
#define HLEVEL3    800 
#define HLEVEL4    750 
#define HLEVEL5    650 
#define HLEVEL6    600 
#define HLEVEL7    550 
#define HLEVEL8    500 
//��ͬʪ�ȶ�Ӧ�Ĳ���
#define KHL1       0.3
#define KHL2       0.35
#define KHL3       0.4
#define KHL4       0.5
#define KHL5       0.6
#define KHL6       0.8
#define KHL7       0.9
#define KHL8       1
#define KHOTHER    1

//�¶ȷֶ�--10��
#define TLEVEL1    550 
#define TLEVEL2    500 
#define TLEVEL3    450 
#define TLEVEL4    400 
#define TLEVEL5    350 
#define TLEVEL6    300 
#define TLEVEL7    250 
#define TLEVEL8    200 
#define TLEVEL9    100 
#define TLEVEL10   50

//��ͬ�¶ȶ�Ӧ�Ĳ���
#define KTL1    0.8
#define KTL2    0.8
#define KTL3    0.8
#define KTL4    0.8
#define KTL5    0.8
#define KTL6    0.8
#define KTL7    0.8
#define KTL8    0.98
#define KTL9    1
#define KTL10   1
#define KTOTHER 1

//��������
#define  STARTAUTOV                 1
#define  STOPAUTOV                  2

// ����ʱ�Զ���ĸ���״̬���� 

#define  STATUS_ALL_STOP            0 
#define  STATUS_SYSINIBEGIN         1 
#define  STATUS_SYSINIEND           2 
#define  STATUS_NETINIBEGIN         3 
#define  STATUS_NETINIEND           4 
#define  STATUS_NETFAIL             5
#define  SYSINIFINISH               6




#define STATUS_AUTOV_START          10 
#define STATUS_CHECK_F1             11
#define STATUS_CHECK_F2             12
#define STATUS_CHECK_F3_L1          13
#define STATUS_CHECK_F3_L2          14
#define STATUS_CHECK_F3_L3          15
#define STATUS_CHECK_F3_L4          16
#define STATUS_CHECK_F3_L5          17
#define STATUS_CHECK_F4             18
#define STATUS_CHECK_OK             19
#define STATUS_CHECK_FAIL           20
#define  STATUS_AUTOV_STOP          21
#define  STATUS_AUTOV_RUN_F1        22
#define  STATUS_AUTOV_RUN_F2        23
#define  STATUS_AUTOV_RUN_F3        24
#define  STATUS_AUTOV_RUN_F3_2MIN   25
#define  STATUS_AUTOV_RUN_F4        26


#define  STATUS_AUTOV_RUN_ON        27
#define  STATUS_AUTOV_RUN_OFF       28
#define  STATUS_AUTOV_WAIT_D1       29
#define  STATUS_AUTOV_WAIT          30
#define  STATUS_AUTOV_STOP_D1       31


extern uint8_t g_status; 
extern uint8_t g_WithoutOnenet;
extern uint8_t g_plant; //ֲ������  0  ����   1 ������
extern uint8_t g_chemical  ; // ��ѧ���� 0��������  1������
extern uint8_t g_checkfreok[4];
extern uint8_t g_touch; //����������־
extern float kp,kv,kc;
extern float g_AutoVtime;
extern uint32_t g_sysruntime ;
void LoadSysPara(void);

#endif

 