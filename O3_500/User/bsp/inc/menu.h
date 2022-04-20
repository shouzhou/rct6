#ifndef __MENU_HH__
#define __MENU_HH__
#include <stdint.h>

extern uint8_t CurMenuIndex;

void  bsp_KeyFunction(uint8_t key);



void switch_menu(uint8_t key);


#endif
