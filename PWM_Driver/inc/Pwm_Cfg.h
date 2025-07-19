#ifndef PWM_CFG_H
#define PWM_CFG_H

#include "Pwm.h"  

/* @define: PWM_NUM_CHANNELS
 * @brief: Số lượng kênh PWM 
 */
#define PWM_NUM_CHANNELS 1  
/* @brief: Callback (prototype)
 * @details: Hàm callback cho kênh 0
 */
extern void Pwm_Channel0_Notification(void);

/* @brief: Bảng cấu hình PWM channels (hardware-specific)
 * @details: Mảng cấu hình các kênh PWM (extern từ main.c)
 */
extern const Pwm_ChannelConfigType PwmChannelsConfig[];

/* @brief: Hàm handler ngắt PWM (prototype)
 * @details: Duyệt config, check và clear flag, gọi callback nếu enable
 */
extern void Pwm_IsrHandler(TIM_TypeDef *TIMx);

/* @brief: Cấu hình tổng cho PWM driver
 * @details: Struct chứa số kênh và mảng channels (extern từ main.c)
 */
extern const Pwm_ConfigType PwmDriverConfig;

// @brief: ISR entry point cho TIM2
// @details: Gọi handler PWM và có thể handler khác nếu chia sẻ ngắt
extern void TIM2_IRQHandler(void);

// @brief: ISR entry point cho TIM3
// @details: Gọi handler PWM cho TIM3
extern void TIM3_IRQHandler(void);

// @brief: ISR entry point cho TIM4
// @details: Gọi handler PWM cho TIM4
extern void TIM4_IRQHandler(void);

#endif /* PWM_CFG_H */