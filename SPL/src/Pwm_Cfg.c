#include "Pwm.h"
#include "Pwm_Cfg.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

/* @brief: Callback (prototype)
 * @details: Hàm callback cho kênh 0
 */
void Pwm_Channel0_Notification(void) {
    GPIOC->ODR ^= GPIO_Pin_13;
}

/* @brief: Hàm handler ngắt PWM (prototype)
 * @details: Duyệt config, check và clear flag, gọi callback nếu enable
 */
void Pwm_IsrHandler(TIM_TypeDef *TIMx) {
    for (uint8_t i = 0; i < PWM_NUM_CHANNELS; ++i) {
        const Pwm_ChannelConfigType *cfg = &PwmChannelsConfig[i];
        if (cfg->TIMx == TIMx) {
            uint16_t flag = TIM_IT_CC1 << (cfg->channel - 1);
            /* Rising edge */
            if (TIM_GetITStatus(TIMx, flag) != RESET) {
                TIM_ClearITPendingBit(TIMx, flag);
                if (cfg->NotificationCb && cfg->notificationEnable) {
                    cfg->NotificationCb(); // Pwm_Channel0_Notification()
                } 
            }
             /* Falling edge */
            if (TIM_GetITStatus(TIMx, TIM_IT_Update) != RESET) {
                TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
                if (cfg->NotificationCb && cfg->notificationEnable) {
                    cfg->NotificationCb();
                }
            }
        }
    }
}

// @brief: ISR entry point cho TIM2
// @details: Gọi handler PWM và có thể handler khác nếu chia sẻ ngắt
void TIM2_IRQHandler(void) {
    Pwm_IsrHandler(TIM2);
}

// @brief: ISR entry point cho TIM3
// @details: Gọi handler PWM và có thể handler khác nếu chia sẻ ngắt
void TIM3_IRQHandler(void) {
    Pwm_IsrHandler(TIM3);
}

// @brief: ISR entry point cho TIM4
// @details: Gọi handler PWM và có thể handler khác nếu chia sẻ ngắt
void TIM4_IRQHandler(void) {
    Pwm_IsrHandler(TIM4);
}