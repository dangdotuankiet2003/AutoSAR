#define PWM_ENABLED  
#define ADC_ENABLED  

#include "IoHwAb.h"
#include "Adc_Cfg.h"
#include "Pwm_Cfg.h"
#include "Pwm.h"
#include "Dio.h"

/* Biến toàn cục lưu cấu hình từ ConfigPtr */
static const IoHwAb_ConfigType *gCfg = NULL_PTR;

/* Biến lưu giá trị ADC raw (mảng để hỗ trợ nhiều kênh) */
static Adc_ValueGroupType adcRawValues[10];
static Adc_GroupDefType adcGroupConfigs[10];

/**
 * @brief: Khởi tạo module IoHwAb
 * @param[in] ConfigPtr: Con trỏ tới cấu hình
 * @details: Cấu hình Port, ADC, PWM, DIO
 */
void IoHwAb_Init(const IoHwAb_ConfigType *ConfigPtr)
{
    if (ConfigPtr == NULL_PTR) {
        return;
    }
    gCfg = ConfigPtr;

    /* Kiểm tra hợp lệ */
    if (gCfg->temperatureSensor.numChannels == 0 ||
        gCfg->temperatureSensor.numChannels > 10 ||
        gCfg->coolingFan.numChannels == 0 ||
        gCfg->coolingFan.numChannels > 4 ||
        DIO_GET_PORT_NUM(gCfg->statusLed.dioChannelId) > DIO_PORT_C ||
        gCfg->coolingFan.TIMx == NULL_PTR) {
        return;
    }

    /* 1. Dynamic Port config */
    Port_PinConfigType portPinCfg[1 + gCfg->temperatureSensor.numChannels + gCfg->coolingFan.numChannels];
    uint8 pinCount = 0;

    /* Cấu hình chân ADC */
    for (uint8 i = 0; i < gCfg->temperatureSensor.numChannels; i++) {
        portPinCfg[pinCount].PortNum = DIO_GET_PORT_NUM(gCfg->temperatureSensor.adcPinIds[i]);
        portPinCfg[pinCount].PinNum = DIO_GET_PIN_NUMBER(gCfg->temperatureSensor.adcPinIds[i]);
        portPinCfg[pinCount].PinMode = PORT_PIN_MODE_ADC;
        portPinCfg[pinCount].PinDirection = PORT_PIN_IN;
        portPinCfg[pinCount].PinPull = 0;
        portPinCfg[pinCount].PinSpeed = 0;
        portPinCfg[pinCount].PinDirectionChangeable = 0;
        portPinCfg[pinCount].PinLevel = PORT_PIN_LEVEL_LOW;
        portPinCfg[pinCount].ModeChangeable = 0;
        pinCount++;
    }

    /* Cấu hình chân PWM */
    for (uint8 i = 0; i < gCfg->coolingFan.numChannels; i++) {
        portPinCfg[pinCount].PortNum = DIO_GET_PORT_NUM(gCfg->coolingFan.pwmPinId[i]);
        portPinCfg[pinCount].PinNum = DIO_GET_PIN_NUMBER(gCfg->coolingFan.pwmPinId[i]);
        portPinCfg[pinCount].PinMode = PORT_PIN_MODE_PWM;
        portPinCfg[pinCount].PinDirection = PORT_PIN_OUT;
        portPinCfg[pinCount].PinPull = 0;
        portPinCfg[pinCount].PinSpeed = 2;
        portPinCfg[pinCount].PinDirectionChangeable = 0;
        portPinCfg[pinCount].PinLevel = PORT_PIN_LEVEL_LOW;
        portPinCfg[pinCount].ModeChangeable = 0;
        pinCount++;
    }

    /* Cấu hình chân LED */
    portPinCfg[pinCount].PortNum = DIO_GET_PORT_NUM(gCfg->statusLed.dioChannelId);
    portPinCfg[pinCount].PinNum = DIO_GET_PIN_NUMBER(gCfg->statusLed.dioChannelId);
    portPinCfg[pinCount].PinMode = PORT_PIN_MODE_DIO;
    portPinCfg[pinCount].PinDirection = PORT_PIN_OUT;
    portPinCfg[pinCount].PinPull = 0;
    portPinCfg[pinCount].PinSpeed = 2;
    portPinCfg[pinCount].PinDirectionChangeable = 0;
    portPinCfg[pinCount].PinLevel = PORT_PIN_LEVEL_HIGH;
    portPinCfg[pinCount].ModeChangeable = 0;
    pinCount++;

    Port_ConfigType portConfig = {
        .PinConfigs = portPinCfg,
        .PinCount = pinCount
    };
    
    Port_Init(&portConfig);

#ifdef ADC_ENABLED
    /* 2. Cấu hình ADC Driver cho cảm biến */
    const Adc_ConfigType* adcConfig = Adc_CreateConfig(gCfg->temperatureSensor.adcInstance, 
                                                      gCfg->temperatureSensor.adcPinIds, 
                                                      gCfg->temperatureSensor.numChannels);
    if (adcConfig != NULL_PTR) {
        Adc_Init(adcConfig, adcGroupConfigs);
        for (uint8 i = 0; i < gCfg->temperatureSensor.numChannels; i++) {
            Adc_SetupResultBuffer(i, &adcRawValues[i]);
        }
    }
#endif

#ifdef PWM_ENABLED
    /* 3. Cấu hình PWM Driver cho quạt */
    const Pwm_ConfigType* pwmConfig = Pwm_CreateConfig( gCfg->coolingFan.TIMx,
                                                         gCfg->coolingFan.pwmPinId,
                                                         gCfg->coolingFan.numChannels,
                                                         gCfg->coolingFan.pwmPeriod,
                                                         gCfg->coolingFan.prescaler,
                                                         gCfg->coolingFan.outputCompareValue,
                                                         gCfg->coolingFan.notificationEnable,
                                                         gCfg->coolingFan.NotificationCb);
    if (pwmConfig != NULL_PTR) {
        Pwm_Init(pwmConfig);
    }
#endif

    /* 4. Cấu hình LED trạng thái  */
    Dio_WriteChannel(gCfg->statusLed.dioChannelId, STD_HIGH);
}

/**
 * @brief: Đọc nhiệt độ từ cảm biến analog
 * @param[in] sensorIndex: Chỉ số cảm biến (0 nếu chỉ có 1 cảm biến)
 * @return: Giá trị nhiệt độ hiện tại (°C) 
 * @details: Gọi ADC driver để đọc và chuyển đổi thành nhiệt độ
 */
uint16 IoHwAb_ReadTemperature(uint8 sensorIndex)
{
#ifdef ADC_ENABLED
    if (gCfg == NULL_PTR || sensorIndex >= gCfg->temperatureSensor.numChannels) {
        if (gCfg->SensorFailedCallback != NULL_PTR) {
            gCfg->SensorFailedCallback();
        }
        return 0;
    }

    Adc_StartGroupConversion(sensorIndex);

    Adc_ValueGroupType adcValue = 0;
    if (Adc_ReadGroup(sensorIndex, &adcValue) != E_OK) {
        if (gCfg->SensorFailedCallback != NULL_PTR) {
            gCfg->SensorFailedCallback();
        }
        return 0;
    }

    /* Chuyển đổi ADC sang nhiệt độ (°C) cho LM35 (fixed-point) */
    uint16 temperature = (adcValue * 330UL) / 4096UL;

    if (temperature > gCfg->temperatureSensor.highThreshold && gCfg->OverheatCallback != NULL_PTR) {
        gCfg->OverheatCallback(temperature);
    }

    return temperature;
#else
    return 0;
#endif
}

/**
 * @brief: Thiết lập tốc độ quạt DC
 * @param[in] percent: Duty cycle (0-100%)
 * @details: Gọi PWM driver để điều chỉnh duty cycle
 */
void IoHwAb_SetFanDuty(uint16 percent)
{
#ifdef PWM_ENABLED
    if (gCfg == NULL_PTR || percent > 100) return;

    uint16 dutyHex = (percent * 0x8000U) / 100U;

    for (uint8 i = 0; i < gCfg->coolingFan.numChannels; i++) {
        Pwm_SetDutyCycle(i, dutyHex);
    }
#endif
}

/**
 * @brief: Điều khiển trạng thái LED
 * @param[in] state: Trạng thái LED 
 * @details: Gọi DIO driver để bật/tắt LED
 */
void IoHwAb_SetLed(bool state)
{
    if (gCfg == NULL_PTR) {
        return;
    }

    if(gCfg->statusLed.dioChannelId == DIO_CHANNEL_C13){
        Dio_LevelType level = (state == true) ? STD_LOW : STD_HIGH;
        Dio_WriteChannel(gCfg->statusLed.dioChannelId, level);
    }
    else{
        Dio_LevelType level = (state == true) ? STD_HIGH : STD_LOW;
        Dio_WriteChannel(gCfg->statusLed.dioChannelId, level);
    }
}