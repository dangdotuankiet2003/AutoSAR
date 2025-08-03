#define PWM_ENABLED  

#include "Dio.h"
#include "Port.h"
#include "Pwm.h"
#include "Pwm_Cfg.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

#ifdef PWM_ENABLED  

/* Biến toàn cục để lưu cấu hình động và số kênh */
static Pwm_ConfigType PwmDynamicConfig;
static Pwm_ChannelConfigType PwmDynamicChannels[4];  // Tối đa 4 kênh PWM 
static uint8 PwmDynamicNumChannels = 0;

/**
 * @brief: Map pwmPinId to channel number for specific TIMx
 * @param[in] pinId: Pin ID from IoHwAb (e.g., DIO_CHANNEL_A6)
 * @param[in] timx: Timer instance (TIM2, TIM3, TIM4)
 * @return: Channel number (1-4) or 0 if invalid
 */
uint8 get_channel_from_pin(Dio_ChannelType pinId, TIM_TypeDef* timx) {
    uint8 port = DIO_GET_PORT_NUM(pinId);
    uint8 pin = DIO_GET_PIN_NUMBER(pinId);
    if (timx == TIM2) {
        if (port == PORT_A && pin == 0) return 1; // PA0: TIM2_CH1
        if (port == PORT_A && pin == 1) return 2; // PA1: TIM2_CH2
        if (port == PORT_A && pin == 2) return 3; // PA2: TIM2_CH3
        if (port == PORT_A && pin == 3) return 4; // PA3: TIM2_CH4
    } else if (timx == TIM3) {
        if (port == PORT_A && pin == 6) return 1; // PA6: TIM3_CH1
        if (port == PORT_A && pin == 7) return 2; // PA7: TIM3_CH2
        if (port == PORT_B && pin == 0) return 3; // PB0: TIM3_CH3
        if (port == PORT_B && pin == 1) return 4; // PB1: TIM3_CH4
    } else if (timx == TIM4) {
        if (port == PORT_B && pin == 6) return 1; // PB6: TIM4_CH1
        if (port == PORT_B && pin == 7) return 2; // PB7: TIM4_CH2
        if (port == PORT_B && pin == 8) return 3; // PB8: TIM4_CH3
        if (port == PORT_B && pin == 9) return 4; // PB9: TIM4_CH4
    }
    return 0; // Invalid pin
}

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
){
    if (numChannels == 0 || numChannels > 4 || TIMx == NULL_PTR || pwmPeriod == 0 || pwmPinId == NULL_PTR) {
        PwmDynamicNumChannels = 0;
        return NULL_PTR;
    }

    PwmDynamicNumChannels = 0;  // Reset and count valid channels

    // Cấu hình động cho từng kênh PWM
    for (uint8 i = 0; i < numChannels; i++) {
        uint8 channel = get_channel_from_pin(pwmPinId[i], TIMx);
        if (channel == 0) continue;  // Nếu chân không hợp lệ, bỏ qua

        PwmDynamicChannels[PwmDynamicNumChannels].TIMx = TIMx;
        PwmDynamicChannels[PwmDynamicNumChannels].channel = channel;
        PwmDynamicChannels[PwmDynamicNumChannels].channelClass = PWM_VARIABLE_PERIOD;
        PwmDynamicChannels[PwmDynamicNumChannels].polarity = PWM_HIGH;
        PwmDynamicChannels[PwmDynamicNumChannels].idleState = PWM_LOW;
        PwmDynamicChannels[PwmDynamicNumChannels].defaultDutyCycle = 0x0000;  
        PwmDynamicChannels[PwmDynamicNumChannels].defaultPeriod = pwmPeriod;
        PwmDynamicChannels[PwmDynamicNumChannels].prescaler = prescaler;
        PwmDynamicChannels[PwmDynamicNumChannels].notificationEnable = notificationEnable ? notificationEnable[i] : 0;
        PwmDynamicChannels[PwmDynamicNumChannels].NotificationCb = NotificationCb ? NotificationCb[i] : NULL_PTR;

        PwmDynamicNumChannels++;  
    }

    PwmDynamicConfig.numChannels = PwmDynamicNumChannels;
    PwmDynamicConfig.channels = PwmDynamicChannels;

    return &PwmDynamicConfig;
}


/**
 * @brief: Lấy số lượng kênh PWM
 * @return: Số kênh PWM hiện tại
 */
uint8 Pwm_GetNumChannels(void)
{
    return PwmDynamicNumChannels;
}

/* @brief: Callback (prototype)
 * @details: Hàm callback cho kênh 0
 */
void Pwm_Channel0_Notification(void){
    GPIOC->ODR ^= GPIO_Pin_13;  // Test LED nếu cần
}

/* @brief: Hàm handler ngắt PWM
 * @details: Duyệt config, check và clear flag, gọi callback nếu enable. Đơn giản hóa để tránh lỗi
 */
void Pwm_IsrHandler(TIM_TypeDef *TIMx) {
    for (uint8_t i = 0; i < PwmDynamicNumChannels; i++) {
        const Pwm_ChannelConfigType *cfg = &PwmDynamicChannels[i];
        if (cfg->TIMx == TIMx && cfg->notificationEnable) {
            uint16_t flag = TIM_IT_CC1 << (cfg->channel - 1);
            if (TIM_GetITStatus(TIMx, flag) != RESET) {
                TIM_ClearITPendingBit(TIMx, flag);
                if (cfg->NotificationCb) cfg->NotificationCb();
            }
            if (TIM_GetITStatus(TIMx, TIM_IT_Update) != RESET) {
                TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
                if (cfg->NotificationCb) cfg->NotificationCb();
            }
        }
    }
}

// @brief: ISR entry point cho TIM2
void TIM2_IRQHandler(void) {
    Pwm_IsrHandler(TIM2);
}

// @brief: ISR entry point cho TIM3
void TIM3_IRQHandler(void) {
    Pwm_IsrHandler(TIM3);
}

// @brief: ISR entry point cho TIM4
void TIM4_IRQHandler(void) {
    Pwm_IsrHandler(TIM4);
}

#endif  // Kết thúc #ifdef PWM_ENABLED