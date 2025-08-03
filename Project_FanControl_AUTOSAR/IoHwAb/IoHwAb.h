#ifndef IOHWAB_H
#define IOHWAB_H

#include "Std_Types.h"
#include "Adc.h"
#include "Pwm.h"
#include "Dio.h"
#include "Port.h"
#include <stdbool.h>

// ---------- Temperature Sensor ----------- //
typedef struct {
    Adc_InstanceType adcInstance;       /* ADC instance (ADC1 hoặc ADC2) */
    Dio_ChannelType adcPinIds[10];      /* Mảng chân ADC (DIO_CHANNEL_A0, A1, ...) */
    uint8 numChannels;                  /* Số kênh ADC (tối đa 10) */
    uint16 minValue;                    /* Giá trị nhiệt độ tối thiểu (°C) */
    uint16 maxValue;                    /* Giá trị nhiệt độ tối đa (°C) */
    uint16 lowThreshold;                /* Ngưỡng nhiệt độ thấp */
    uint16 highThreshold;               /* Ngưỡng nhiệt độ cao */
} IoHwAb_TemperatureSensorType;

// ---------- Cooling Fan (PWM) ----------- //
typedef struct {
    Dio_ChannelType pwmPinId[4];        /* Mảng chân PWM (DIO_CHANNEL_A6, ...) */
    uint8 numChannels;                  /* Số kênh PWM (1-4) */
    uint8 notificationEnable[4];        /* Bật thông báo cho từng kênh */
    void (*NotificationCb[4])(void);    /* Callback cho từng kênh */
    TIM_TypeDef* TIMx;                  /* Timer sử dụng PWM */
    uint16 outputCompareValue;          /* Giá trị output compare mặc định */
    uint16 pwmPeriod;                   /* Chu kỳ PWM (ticks) */
    uint16 prescaler;                   /* Prescaler cho timer */
} IoHwAb_CoolingFanType;

// ---------- LED status ----------- //
typedef struct {
    Dio_ChannelType dioChannelId;       /* Chân DIO cho LED */
} IoHwAb_StatusLedType;

typedef struct {
    IoHwAb_TemperatureSensorType temperatureSensor;  /* Cấu hình cảm biến nhiệt độ */
    IoHwAb_CoolingFanType coolingFan;               /* Cấu hình quạt */
    IoHwAb_StatusLedType statusLed;                 /* Cấu hình LED */
    void (*OverheatCallback)(uint16);               /* Callback khi quá nhiệt */
    void (*SensorFailedCallback)(void);             /* Callback khi cảm biến lỗi */
} IoHwAb_ConfigType;

/**
 * @brief: Khởi tạo module IoHwAb
 * @param[in] ConfigPtr: Con trỏ tới cấu trúc chứa thông tin cấu hình
 * @details: Cấu hình các driver MCAL (Port, ADC, PWM, DIO)
 */
void IoHwAb_Init(const IoHwAb_ConfigType *ConfigPtr);

/**
 * @brief: Đọc nhiệt độ từ cảm biến analog
 * @param[in] sensorIndex: Chỉ số cảm biến (0 nếu chỉ có 1 cảm biến)
 * @return: Giá trị nhiệt độ hiện tại (°C) kiểu uint16
 * @details: Gọi ADC driver để đọc và chuyển đổi thành nhiệt độ
 */
uint16 IoHwAb_ReadTemperature(uint8 sensorIndex);

/**
 * @brief: Thiết lập tốc độ quạt DC
 * @param[in] percent: Duty cycle PWM (0-100%)
 * @details: Gọi PWM driver để điều chỉnh duty cycle
 */
void IoHwAb_SetFanDuty(uint16 percent);

/**
 * @brief: Điều khiển trạng thái LED
 * @param[in] state: Trạng thái LED (TRUE: bật, FALSE: tắt)
 * @details: Gọi DIO driver để bật/tắt LED
 */
void IoHwAb_SetLed(bool state);

#endif