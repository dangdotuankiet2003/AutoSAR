#ifndef ADC_CFG_H
#define ADC_CFG_H

#include "Adc.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"

#define ADC_NUM_GROUPS 3

extern const uint8 Adc_Channel_Mapping[10];
extern uint8 adcInit;

Std_ReturnType Adc_EnableDma(Adc_GroupType Group);
Std_ReturnType Adc_DisableDma(Adc_GroupType Group);
void ADC1_2_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
Std_ReturnType Adc_SetupResultBuffer_Dma(Adc_GroupType Group, Adc_ValueGroupType* buf);

#endif // ADC_CFG_H