#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"

typedef uint8 Pwm_ChannelType;
typedef uint32 Pwm_PeriodType;

typedef enum {
    PWM_HIGH ,     /* Đầu ra ở mức cao */
    PWM_LOW        /* Đầu ra ở mức thấp*/
} Pwm_OutputStateType;

typedef enum {
    PWM_RISING_EDGE,   /* Cạnh lên */
    PWM_FALLING_EDGE ,  /* Cạnh xuống */
    PWM_BOTH_EDGES      /* Cả hai cạnh */
} Pwm_EdgeNotificationType;

typedef enum {
    PWM_VARIABLE_PERIOD ,         /* Cho phép Duty circle và chu kì của kênh có thể thay đổi */
    PWM_FIXED_PERIOD ,            /* Chu kỳ của kênh cố định, chỉ có thể thay đổi Duty cycle */
    PWM_FIXED_PERIOD_SHIFTED     /* Chu kỳ của kênh cố định nhưng cho phép dịch pha (phase shift) giữa các kênh PWM */
} Pwm_ChannelClassType;

typedef struct {
    TIM_TypeDef* TIMx;                   /* Con trỏ đến timer base (ví dụ: TIM2, TIM3) */
    uint8 channel;                       /* Số kênh (1-4 trên STM32) */
    Pwm_ChannelClassType channelClass;   /* Lớp kênh (variable/fixed/shifted) */
    Pwm_OutputStateType polarity;        /* Polarity (PWM_HIGH hoặc PWM_LOW) */
    Pwm_OutputStateType idleState;       /* Trạng thái idle (high/low) */
    uint16 defaultDutyCycle;             /* Duty cycle mặc định (0x0000 đến 0x8000) */
    Pwm_PeriodType defaultPeriod;        /* Chu kỳ mặc định (ticks) */
    uint16 prescaler;                    /* Prescaler cho timer */
    uint8 notificationEnable;            /* Enable thông báo ngắt (0/1) */
    void (*NotificationCb)(void);        /* Callback cho thông báo */
} Pwm_ChannelConfigType;

typedef struct {
    uint8_t numChannels;                     /* Số lượng kênh PWM */
    const Pwm_ChannelConfigType* channels;  /* Mảng cấu hình các kênh */
} Pwm_ConfigType;

/**
 * @brief: Hàm khởi tạo PWM driver
 * @param[in] ConfigPtr: Con trỏ đến cấu hình PWM
 * @details: Khởi tạo biến nội bộ và phần cứng PWM theo cấu hình
 */
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);

/**
 * @brief: Hàm hủy khởi tạo PWM driver
 * @details: Đặt PWM về trạng thái ban đầu, tắt ngắt và thông báo
 */
void Pwm_DeInit(void);

/**
 * @brief: Hàm đặt duty cycle cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] DutyCycle: Giá trị duty cycle (0x0000: 0%, 0x8000: 100%)
 * @details: Cập nhật duty cycle mà không thay đổi chu kỳ
 */
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle);

/**
 * @brief: Hàm đặt chu kỳ và duty cycle cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] Period: Giá trị chu kỳ (ticks)
 * @param[in] DutyCycle: Giá trị duty cycle (0x0000: 0%, 0x8000: 100%)
 * @details: Chỉ áp dụng cho Class type PWM_VARIABLE_PERIOD
 */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16 DutyCycle);

/**
 * @brief: Hàm đặt đầu ra PWM về trạng thái idle
 * @param[in] ChannelNumber: Số kênh PWM
 * @details: Đặt đầu ra về trạng thái idle đã cấu hình (high/low)
 */
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);

/**
 * @brief: Hàm lấy trạng thái đầu ra của PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @return: Trạng thái đầu ra (PWM_HIGH / PWM_LOW)
 * @details: Trả về mức logic hiện tại của đầu ra PWM
 */
Pwm_OutputStateType Pwm_GetOutputState (Pwm_ChannelType ChannelNumber);

/**
 * @brief: Hàm tắt thông báo ngắt cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @details: Tắt thông báo cạnh cho kênh cụ thể
 */
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber);

/**
 * @brief: Hàm bật thông báo ngắt cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] Notification: Loại cạnh thông báo (PWM_RISING_EDGE, PWM_FALLING_EDGE, PWM_BOTH_EDGES)
 * @details: Bật thông báo cạnh cho kênh cụ thể
 */
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification);

/**
 * @brief: Hàm lấy thông tin phiên bản PWM driver
 * @param[out] versioninfo: Con trỏ đến cấu trúc chứa thông tin phiên bản
 * @details: Trả về phiên bản module, ...
 */
void Pwm_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* PWM_H */