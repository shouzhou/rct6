#ifndef _USART_H
#define _USART_H
#include <stm32f10x.h>
#ifdef __cplusplus
extern "C"  {
#endif
extern void usart(u16 i);
extern void USART1_Configuration(void);
extern void USART3_Configuration(void);
extern void data_init(void);           //D8000~D8126≥ı ºªØ
#ifdef __cplusplus
}
#endif

#endif /*_USART_H*/
