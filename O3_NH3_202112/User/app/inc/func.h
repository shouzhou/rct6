

#ifndef __APP_FUNC_H
#define __APP_FUNC_H

#include "stdint.h"

//������ �������� ��Ӧ�İ�ť״̬���� ����ʱ �᷵�ض�Ӧ������
//0	�����ֶ�ģʽ	0x000A	1	�����ֶ�ģʽ			0
//0	����ѭ��ģʽ	0x000A	1	����ѭ��ģʽ			1
//4	�����Զ�Ũ��	0x000A	1	�����Զ�Ũ��			3
//4	ֹͣ�Զ�Ũ��	0x000A	1	ֹͣ�Զ�Ũ��			4
//4	�����Զ����	0x000A	1	�����Զ����			5
//4	ֹͣ�Զ����	0x000A	1	ֹͣ�Զ����			6
//3	ֹͣ�ֶ�ģʽ	0x000A	1	ֹͣ�ֶ�ģʽ			2
//5	ֹͣѭ��ģʽ	0x000A	1	ֹͣѭ��ģʽ			7
#define  STARTMANUAL   0
#define  STARTLOOP     1
#define  STOPMANUAL    2
#define  STARTAUTODEN  3
#define  STOPAUTODEN   4
#define  STARTAUTOV    5
#define  STOPAUTOV     6
#define  STOPLOOP      7


// ����ʱ�Զ���ĸ���״̬���� 
#define  STATUS_ALL_STOP         0 
#define  STATUS_MANUAL_START     1
#define  STATUS_MANUAL_STOP      2
#define  STATUS_MANUAL_RUN_D1    10
#define  STATUS_MANUAL_STOP_D1   11 //�ӳٹرս׶�

#define  STATUS_LOOP_START       3 
#define  STATUS_LOOP_STOP        4 
#define  STATUS_LOOP_RUN_F1      30  //���п��׶�
#define  STATUS_LOOP_RUN_F2      31  //���йؽ׶�
#define  STATUS_LOOP_STOP_D1     32 //�ӳٹرս׶�


#define  STATUS_AUTODEN_START       5 
#define  STATUS_AUTODEN_STOP        6 
#define  STATUS_AUTODEN_RUN_F1      50  //�Զ�Ũ��ģʽ��һŨ�Ƚ׶�
#define  STATUS_AUTODEN_RUN_F2      51  //�Զ�Ũ��ģʽ�ڶ�Ũ�Ƚ׶�
#define  STATUS_AUTODEN_RUN_F3      52  //�Զ�Ũ��ģʽ����Ũ�Ƚ׶�
#define  STATUS_AUTODEN_RUN_F4      53  //�Զ�Ũ��ģʽ����Ũ�Ƚ׶�
#define  STATUS_AUTODEN_RUN_F4_ON   54  //�Զ�Ũ��ģʽ����Ũ�Ƚ׶�_��
#define  STATUS_AUTODEN_RUN_F4_OFF  55  //�Զ�Ũ��ģʽ����Ũ�Ƚ׶�_��
#define  STATUS_AUTODEN_WAIT_D1     56  //�����ӳٽ׶�
#define  STATUS_AUTODEN_STOP_D1     57  //�ӳٹرս׶�
#define  STATUS_AUTODEN_WAIT        58  //�����׶�



#define  STATUS_AUTOV_START     7 
#define  STATUS_AUTOV_STOP      8
#define  STATUS_AUTOV_RUN_F1    71
#define  STATUS_AUTOV_RUN_F2    72
#define  STATUS_AUTOV_RUN_F3    73
#define  STATUS_AUTOV_RUN_F4    74
#define  STATUS_AUTOV_WAIT_D1   75
#define  STATUS_AUTOV_WAIT      76
#define  STATUS_AUTOV_STOP_D1   77

#define  STATUS_AUTOV_RUN_ON    79
#define  STATUS_AUTOV_RUN_OFF   80



#define LOWMAXTIME      300   //�޸�����꣬�ı��Ũ��ʱ���Ϲص���ʱ��
#define LOWSWITCHTIME   30    //�޸�����꣬�ı��Ũ��ʱÿ�ο��͹ص�ʱ��
#define FANOFFTIME      30   //�޸�����꣬�ı����ӳٹر�ʱ��
#define AUTOVONTIME    30     //�޸�����꣬�ı��Զ����ģʽ���𵴿�ʱ��
#define AUTOVOFFTIME   30     //�޸�����꣬�ı��Զ����ģʽ���𵴹�ʱ��

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




//void FuncManual(uint16_t mode);
//void FuncAutoV(uint16_t mode);
//void FuncAutoNH3(uint16_t mode);
//void FuncLoop(uint16_t mode);
void FunSwitch(void);
void FunFanOpen(void);
void FunFanClose(void);
void FunO3Control(uint8_t onnum,uint8_t index);
float FunO3CalParameter(void);
extern uint8_t g_status ;


#endif

 
