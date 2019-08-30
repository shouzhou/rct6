#include "bsp.h"
#define SIZEOFMENU 30
uint8_t CurMenuIndex=1;	//ָ��ǰ�Ĳ˵����
//uint8_t lcdflashflag=1;//��־lcdˢ�� =1��ʾ��Ҫˢ��

uint8_t  LocationIndex=1;  //ָʾ��ǰ�Ĳ˵�����

typedef struct keymenu
{  	
	uint8_t curindex;//��ǰ�Ĳ˵����
	uint8_t keyDNindex;//�������°��� ����ı��
	uint8_t keyUPindex;//�������ϰ��� ����ı��
	uint8_t keyENTERindex;//���»س����� ����ı��
	uint8_t keyCANCELindex;//���·��ذ��� ����ı��
	//void (*operation)(uint8_t key);

} MYMENU;

MYMENU  keytab[SIZEOFMENU] =
{
   //��ǰ�� �£�   �ϣ�  �س��� ����    ���ú���
     {0,     1,     1,     1,     1 }, // �˵���1�ſ�ʼ �����0 ��ʼ �����
     {1,     2,     1,    3,      1 },
     {2,     2,     1,    6,      2 },
     
     {3,     4,     3,    9,      1 },
     {4,     5,     3,    10,     1 },
     {5,     5,     4,    11,     1 },
     
     {6,     7,     6,    6,      6 },
     {7,     8,     6,    7,      7 },
     {8,     8,     7,    2,      8 },
    
     {9,     9,     9,    3,      3 },
     {10,    10,    10,   4,      4 },
     {11,    11,    11,   5,      5 },
     
     


};


// A   �� 
// B   ��
// C   ȡ��
// D   ȷ��

void switch_menu(uint8_t key)
{
     uint8_t temp = key;
      switch(temp)
  		
		{
		 	case KEYPAD_D:    //���°���������
               
			   CurMenuIndex= keytab[CurMenuIndex].keyENTERindex;
			  
               
			   break;
		    case KEYPAD_C:
				CurMenuIndex= keytab[CurMenuIndex].keyCANCELindex;
				
			   break;
		   	case KEYPAD_A:
				CurMenuIndex= keytab[CurMenuIndex].keyUPindex;
				
			   break;
		    case KEYPAD_B:
				CurMenuIndex= keytab[CurMenuIndex].keyDNindex;
				
			   break;
           
			default :
			    
			    break;
   		}
      
	  // keytab[CurMenuIndex].operation(temp);
        #ifdef USEDEBUG
        printf("CurMenuIndex = %d\r\n",CurMenuIndex);
        
        #endif
        bsp_Diwen_Updatedata(0X0008,CurMenuIndex); //��Ӧ��Ļ�еı���ͼ����ʾ�����ĸ��˵�����Ӧ�ı���ͼ�����ʾ�����������ڽ���
}



void  bsp_KeyFunction(uint8_t key)
{
    uint8_t temp;
    temp = key;
    if(CurMenuIndex==1)  //��һ������
    {
      switch(temp)
      {
     
            case  KEYPAD_D :
                bsp_SendKey(0x01);
                printf("keyD press\r\n");
                break;
            case  KEYPAD_1 :
                bsp_IOOn(1);
                bsp_IOOn(2);
                bsp_IOOn(3);
                bsp_IOOn(4);
                bsp_IOOn(5);
            
                break;
            case  KEYPAD_2 :
                bsp_IOOff(1);
                bsp_IOOff(2);
                bsp_IOOff(3);
                bsp_IOOff(4);
                bsp_IOOff(5);
                break;
            default :
                break;
      }          
        
    }
    else if(CurMenuIndex ==2) 
    {
       switch(temp)
      {
            
            case  KEYPAD_D :
                bsp_SendKey(0x02);
                printf("keyD press\r\n");
                break;
            default :
                break;
      }        
    }
    else if(CurMenuIndex ==3)  //���������
    {
       switch(temp)
      {
            case  KEYPAD_D :
                bsp_SendKey(0x03);  //3��Ӧ���Ȱ���
               // printf("keyD press\r\n");
                break;
            case  KEYPAD_C :
                bsp_SendKey(0x06);  //����
               // printf("keyD press\r\n");
                break;
            default :
                break;
      } 
    }
    else if(CurMenuIndex ==4)  //���������
    {
         switch(temp)
      {
            
            case  KEYPAD_A :
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
            case  KEYPAD_B:
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
           
            case  KEYPAD_D :
                bsp_SendKey(0x04);  //4 ���ÿ�Ȱ���
               // printf("keyD press\r\n");
                break;
            case  KEYPAD_C :
                bsp_SendKey(0x06);  //����
               // printf("keyD press\r\n");
            default :
                break;
      }
    }
    else if(CurMenuIndex ==5)  //�߶��������
    {
         switch(temp)
      {
            
            case  KEYPAD_A :
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
            case  KEYPAD_B:
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
           
            case  KEYPAD_D :
                bsp_SendKey(0x05);  //5��Ӧ�߶��������
               // printf("keyD press\r\n");
                break;
            case  KEYPAD_C :
                bsp_SendKey(0x06);  //����
               // printf("keyD press\r\n");
            default :
                break;
      }
    }
    
    else if(CurMenuIndex ==6)
    {
         switch(temp)
      {
            
            case  KEYPAD_A :
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
            case  KEYPAD_B:
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
           
            case  KEYPAD_D :
                bsp_SendKey(0x08);  //6��Ӧ����
               // printf("keyD press\r\n");
                break;
            
            default :
                break;
      }
    }
    else if(CurMenuIndex ==7)
    {
          switch(temp)
      {
            
            case  KEYPAD_A :
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
            case  KEYPAD_B:
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
           
            case  KEYPAD_D :
                bsp_SendKey(0x09);  //7��Ӧֹͣ
               // printf("keyD press\r\n");
                break;
            
            default :
                break;
      }
    }
    else if(CurMenuIndex ==8)
    {
         switch(temp)
      {
            
            case  KEYPAD_A :
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
            case  KEYPAD_B:
               // bsp_SendKey(0x0A);
               // printf("keyB press\r\n");
                break;
           
            case  KEYPAD_D :
                bsp_SendKey(0x0A);  //8��Ӧ����
               // printf("keyD press\r\n");
                break;
            
            default :
                break;
      } 
    }
    
    
    else if(CurMenuIndex ==9)
    {
      switch(temp)
      {
            
            case  KEYPAD_0 :
                bsp_SendKey(0x30); 
                break;
            case  KEYPAD_1:
               bsp_SendKey(0x31); 
                break;
           
            case  KEYPAD_2 :
                bsp_SendKey(0x32);  
                         
                break;
            case  KEYPAD_3 :
                bsp_SendKey(0x33); 
                break;
            case  KEYPAD_4:
               bsp_SendKey(0x34); 
                break;
           
            case  KEYPAD_5 :
                bsp_SendKey(0x35);  
                         
                break;
            case  KEYPAD_6 :
                bsp_SendKey(0x36); 
                break;
            case  KEYPAD_7:
               bsp_SendKey(0x37); 
                break;
           
            case  KEYPAD_8 :
                bsp_SendKey(0x38);  
                         
                break;
            case  KEYPAD_9 :
                bsp_SendKey(0x39); 
                break;
            case  KEYPAD_D:  // ok ��ӦD
               bsp_SendKey(60); 
                break;
           
            case  KEYPAD_C :  // ESC ��Ӧ C
                bsp_SendKey(59);  
                         
                break;
            case  KEYPAD_STAR : // .
                bsp_SendKey(47);  
                         
                break;
            case  KEYPAD_HASH : // <<   ��Ӧ#
                bsp_SendKey(58);  
                         
                break;
            default :
                break;
      }
    }
    else if(CurMenuIndex ==10)
    {
         switch(temp)
      {
            
            case  KEYPAD_0 :
                bsp_SendKey(0x30); 
                break;
            case  KEYPAD_1:
               bsp_SendKey(0x31); 
                break;
           
            case  KEYPAD_2 :
                bsp_SendKey(0x32);  
                         
                break;
            case  KEYPAD_3 :
                bsp_SendKey(0x33); 
                break;
            case  KEYPAD_4:
               bsp_SendKey(0x34); 
                break;
           
            case  KEYPAD_5 :
                bsp_SendKey(0x35);  
                         
                break;
            case  KEYPAD_6 :
                bsp_SendKey(0x36); 
                break;
            case  KEYPAD_7:
               bsp_SendKey(0x37); 
                break;
           
            case  KEYPAD_8 :
                bsp_SendKey(0x38);  
                         
                break;
            case  KEYPAD_9 :
                bsp_SendKey(0x39); 
                break;
            case  KEYPAD_D:  // ok ��ӦD
               bsp_SendKey(60); 
                break;
           
            case  KEYPAD_C :  // ESC ��Ӧ C
                bsp_SendKey(59);  
                         
                break;
            case  KEYPAD_STAR : // .
                bsp_SendKey(47);  
                         
                break;
            case  KEYPAD_HASH : // <<   ��Ӧ#
                bsp_SendKey(58);  
                         
                break;
            default :
                break;
      }
    }
    else if(CurMenuIndex ==11)
    {
         switch(temp)
      {
            
            case  KEYPAD_0 :
                bsp_SendKey(0x30); 
                break;
            case  KEYPAD_1:
               bsp_SendKey(0x31); 
                break;
           
            case  KEYPAD_2 :
                bsp_SendKey(0x32);  
                         
                break;
            case  KEYPAD_3 :
                bsp_SendKey(0x33); 
                break;
            case  KEYPAD_4:
               bsp_SendKey(0x34); 
                break;
           
            case  KEYPAD_5 :
                bsp_SendKey(0x35);  
                         
                break;
            case  KEYPAD_6 :
                bsp_SendKey(0x36); 
                break;
            case  KEYPAD_7:
               bsp_SendKey(0x37); 
                break;
           
            case  KEYPAD_8 :
                bsp_SendKey(0x38);  
                         
                break;
            case  KEYPAD_9 :
                bsp_SendKey(0x39); 
                break;
            case  KEYPAD_D:  // ok ��ӦD
               bsp_SendKey(60); 
                break;
           
            case  KEYPAD_C :  // ESC ��Ӧ C
                bsp_SendKey(59);  
                         
                break;
            case  KEYPAD_STAR : // .
                bsp_SendKey(47);  
                         
                break;
            case  KEYPAD_HASH : // <<   ��Ӧ#
                bsp_SendKey(58);  
                         
                break;
            default :
                break;
      }
    }
}

