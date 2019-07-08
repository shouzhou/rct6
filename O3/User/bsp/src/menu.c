#include "bsp.h"
#define SIZEOFMENU 30
uint8_t CurMenuIndex=1;	//指向当前的菜单编号
//uint8_t lcdflashflag=1;//标志lcd刷新 =1表示需要刷新

uint8_t  LocationIndex=1;  //指示当前的菜单焦点

typedef struct keymenu
{  	
	uint8_t curindex;//当前的菜单编号
	uint8_t keyDNindex;//按下向下按键 进入的编号
	uint8_t keyUPindex;//按下向上按键 进入的编号
	uint8_t keyENTERindex;//按下回车按键 进入的编号
	uint8_t keyCANCELindex;//按下返回按键 进入的编号
	//void (*operation)(uint8_t key);

} MYMENU;

MYMENU  keytab[SIZEOFMENU] =
{
   //当前， 下，   上，  回车， 返回    调用函数
     {0,     1,     1,     1,     1 }, // 菜单从1号开始 数组从0 开始 搞错了
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


// A   上 
// B   下
// C   取消
// D   确认

void switch_menu(uint8_t key)
{
     uint8_t temp = key;
      switch(temp)
  		
		{
		 	case KEYPAD_D:    //向下按键被按下
               
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
        bsp_Diwen_Updatedata(0X0008,CurMenuIndex); //对应屏幕中的变量图标显示，在哪个菜单，对应的变量图标就显示，作用类似于焦点
}



void  bsp_KeyFunction(uint8_t key)
{
    uint8_t temp;
    temp = key;
    if(CurMenuIndex==1)  //第一个界面
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
    else if(CurMenuIndex ==3)  //长输入进入
    {
       switch(temp)
      {
            case  KEYPAD_D :
                bsp_SendKey(0x03);  //3对应长度按键
               // printf("keyD press\r\n");
                break;
            case  KEYPAD_C :
                bsp_SendKey(0x06);  //返回
               // printf("keyD press\r\n");
                break;
            default :
                break;
      } 
    }
    else if(CurMenuIndex ==4)  //宽输入进入
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
                bsp_SendKey(0x04);  //4 对用宽度按键
               // printf("keyD press\r\n");
                break;
            case  KEYPAD_C :
                bsp_SendKey(0x06);  //返回
               // printf("keyD press\r\n");
            default :
                break;
      }
    }
    else if(CurMenuIndex ==5)  //高度输入进入
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
                bsp_SendKey(0x05);  //5对应高度输入进入
               // printf("keyD press\r\n");
                break;
            case  KEYPAD_C :
                bsp_SendKey(0x06);  //返回
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
                bsp_SendKey(0x08);  //6对应启动
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
                bsp_SendKey(0x09);  //7对应停止
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
                bsp_SendKey(0x0A);  //8对应返回
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
            case  KEYPAD_D:  // ok 对应D
               bsp_SendKey(60); 
                break;
           
            case  KEYPAD_C :  // ESC 对应 C
                bsp_SendKey(59);  
                         
                break;
            case  KEYPAD_STAR : // .
                bsp_SendKey(47);  
                         
                break;
            case  KEYPAD_HASH : // <<   对应#
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
            case  KEYPAD_D:  // ok 对应D
               bsp_SendKey(60); 
                break;
           
            case  KEYPAD_C :  // ESC 对应 C
                bsp_SendKey(59);  
                         
                break;
            case  KEYPAD_STAR : // .
                bsp_SendKey(47);  
                         
                break;
            case  KEYPAD_HASH : // <<   对应#
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
            case  KEYPAD_D:  // ok 对应D
               bsp_SendKey(60); 
                break;
           
            case  KEYPAD_C :  // ESC 对应 C
                bsp_SendKey(59);  
                         
                break;
            case  KEYPAD_STAR : // .
                bsp_SendKey(47);  
                         
                break;
            case  KEYPAD_HASH : // <<   对应#
                bsp_SendKey(58);  
                         
                break;
            default :
                break;
      }
    }
}

