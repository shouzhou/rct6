

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

void ADC1_GPIO_Configuration(void);
void ADC1_DMA_RCC_Configuration(void);
void ADC1_DMA_Init(void); 

extern __IO uint16_t ADCConvertedValue[4];

#endif

/*****************************    *********************************/
