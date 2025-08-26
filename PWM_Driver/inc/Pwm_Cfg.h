#ifndef PWM_CFG_H
#define PWM_CFG_H

#include "Pwm.h"  
#include "Dio.h"

#ifdef PWM_ENABLED  

/**
 * @brief: Map pwmPinId to channel number for specific TIMx
 * @param[in] pinId: Pin ID from IoHwAb (e.g., DIO_CHANNEL_A6)
 * @param[in] timx: Timer instance (TIM2, TIM3, TIM4)
 * @return: Channel number (1-4) or 0 if invalid
 */
uint8 get_channel_from_pin(Dio_ChannelType pinId, TIM_TypeDef* timx);

/**
 * @brief: Khởi tạo cấu hình PWM động theo tham số truyền vào
 * @param[in] timx: Timer instance (TIM2, TIM3, hoặc TIM4)
 * @param[in] numChannels: Số kênh PWM cần cấu hình (1-4)
 * @param[in] period: Chu kỳ PWM (ticks)
 * @param[in] prescaler: Prescaler cho timer
 * @param[in] pwmPinIds: Mảng các chân PWM
 * @return: Con trỏ tới cấu hình PWM động
 * @details: Dynamic fill Pwm_ChannelConfigType giống code snippet 
 */
const Pwm_ConfigType* Pwm_CreateConfig(
    TIM_TypeDef* TIMx,
    Dio_ChannelType* pwmPinId,
    uint8 numChannels,
    uint16 pwmPeriod,
    uint16 prescaler,
    uint16 outputCompareValue,
    const uint8* notificationEnable,
    const void (*NotificationCb[])(void)
);

/**
 * @brief: Lấy số lượng kênh PWM
 * @return: Số kênh PWM hiện tại
 */
uint8 Pwm_GetNumChannels(void);

/* @brief: Callback (prototype)
 * @details: Hàm callback cho kênh 0
 */
extern void Pwm_Channel0_Notification(void);

/* @brief: Hàm handler ngắt PWM (prototype)
 * @details: Duyệt config, check và clear flag, gọi callback nếu enable
 */
extern void Pwm_IsrHandler(TIM_TypeDef *TIMx);

// @brief: ISR entry point cho TIM2
// @details: Gọi handler PWM và có thể handler khác nếu chia sẻ ngắt
extern void TIM2_IRQHandler(void);

// @brief: ISR entry point cho TIM3
// @details: Gọi handler PWM cho TIM3
extern void TIM3_IRQHandler(void);

// @brief: ISR entry point cho TIM4
// @details: Gọi handler PWM cho TIM4
extern void TIM4_IRQHandler(void);

#endif  // Kết thúc #ifdef PWM_ENABLED

#endif /* PWM_CFG_H */
