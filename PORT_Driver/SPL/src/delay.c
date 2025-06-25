#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

// Hàm khởi tạo Timer 2 (TIM2) cho chức năng delay
void TIM_Config(void)
{
    // Kích hoạt xung nhịp cho TIM2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // Cấu hình Timer
    TIM_TimeBaseInitTypeDef tim;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;    // Không chia tần số
    tim.TIM_CounterMode = TIM_CounterMode_Up; // Đếm lên
    tim.TIM_Period = 0xFFFF;                 // Chu kỳ tối đa
    tim.TIM_Prescaler = 8000 - 1;            // Chia tần số: 8MHz / 8000 = 1kHz (1ms/tick)
    TIM_TimeBaseInit(TIM2, &tim);

    // Kích hoạt Timer
    TIM_Cmd(TIM2, ENABLE);
}


void Delay_ms(uint32_t ms)
{
    if (ms == 0) return;
    static uint8_t isInitialized = 0;
    if (!isInitialized) {
        TIM_Config();
        isInitialized = 1;
    }
    while (ms > 0xFFFF) {
        // Đặt lại bộ đếm
        TIM_SetCounter(TIM2, 0);
        while (TIM_GetCounter(TIM2) < 0xFFFF);
        
        ms -= 0xFFFF; 
    }

    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < ms);
}