

#ifndef __BSP_IOCONTROL_H
#define __BSP_IOCONTROL_H

/* ���ⲿ���õĺ������� */
void bsp_InitIO(void);
void bsp_IOOn(uint8_t _no);
void bsp_IOOff(uint8_t _no);
void bsp_IOToggle(uint8_t _no);
uint8_t bsp_IsIOOn(uint8_t _no);

#endif

 
