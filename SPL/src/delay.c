#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

void Delay(uint32_t ms) {
    uint32_t i;
    for (i = 0; i < ms * 8000; i++);
}