#ifndef DELAY_H_
#define DELAY_H_
#include "stdint.h"

/* @func: TIM_Config
 * @brief: Khởi tạo Timer 2 (TIM2) để sử dụng cho chức năng delay
 * @details: Cấu hình TIM2 với tần số tick 1kHz (1ms) 
 */
void TIM_Config(void);

/* @func: Delay_ms
 * @brief: Hàm tạo độ trễ theo mili giây
 * @details: Sử dụng TIM2 để tạo độ trễ
 */
void Delay(uint32_t ms);

#endif
