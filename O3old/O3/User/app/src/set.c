#include "bsp.h"
uint8_t g_status =0;   //ϵͳ���е�����״̬�л�
uint8_t g_WithoutOnenet=0; //��ʹ��onenet ��־  =1 ��ʾ ��ʹ��ONENET
uint8_t g_plant=0; //ֲ������  0  ����   1 ������
uint8_t g_chemical =0 ; // ��ѧ���� 0��������  1������
uint8_t g_checkfreok[4]={0,0,0,0};
uint8_t g_touch=0xff; //���������־
float kv = 0.75; //�������
float kc = 1.0; //����ʱ�����  kc*��� 
float kp = 1.0; //����Ĳ���ʱ�� kh*kt
uint32_t g_sysruntime =0;
void LoadSysPara(void)
{
    
}