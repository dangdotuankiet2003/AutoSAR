#include "IoHwAb.h"
#include "Pwm_Cfg.h"
#include "delay.h"
#include "uart_display.h"
#include "stm32f10x.h"

// Callback khi quá nhiệt
void OverheatCallback(uint16 currentTemp){
    IoHwAb_SetFanDuty(100); // Bật quạt tối đa
}

// Callback khi sensor fail
void SensorFailedCallback(void){
    IoHwAb_SetLed(true);  // Bật LED cảnh báo
}

// Port config cho UART (PA9, PA10)
const Port_PinConfigType PortPinsConfig[] = {
    // Pin PA9 cho UART1 TX
    {
        .PortNum = PORT_A,
        .PinNum = 9,
        .PinMode = PORT_PIN_MODE_UART,
        .PinDirection = PORT_PIN_OUT,
        .PinPull = 0,
        .PinSpeed = 2,
        .PinLevel = PORT_PIN_LEVEL_HIGH,
        .PinDirectionChangeable = 0,
        .ModeChangeable = 0
    },
    // Pin PA10 cho UART1 RX
    {
        .PortNum = PORT_A,
        .PinNum = 10,
        .PinMode = PORT_PIN_MODE_UART,
        .PinDirection = PORT_PIN_IN,
        .PinPull = 0,
        .PinSpeed = 2,
        .PinLevel = PORT_PIN_LEVEL_LOW,
        .PinDirectionChangeable = 0,
        .ModeChangeable = 0
    }
};

const Port_ConfigType PortDriverConfig = {
    .PinCount = 2,
    .PinConfigs = PortPinsConfig
};

int main(void){
    SystemInit();

    // Cấu hình IoHwAb
    IoHwAb_ConfigType myConfig = {
        .temperatureSensor = {
            .adcInstance = ADC_INSTANCE_1,
            .adcPinIds = {DIO_CHANNEL_A0},   // LM35 -> PA0
            .numChannels = 1,
            .minValue = 0,
            .maxValue = 100,
            .lowThreshold = 30,
            .highThreshold = 40
        },
        .coolingFan = {
            .TIMx = TIM3,
            .pwmPinId = {DIO_CHANNEL_A6, 0, 0, 0},  // PA6 (TIM3_CH1)
            .numChannels = 1,
            .pwmPeriod = 100,
            .prescaler = 71,
            .outputCompareValue = 0,
            .notificationEnable = {0, 0, 0, 0},  // Tắt notification
            .NotificationCb = {NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR}
        },
        .statusLed = {
            .dioChannelId = DIO_CHANNEL_A3
        },
        .OverheatCallback = OverheatCallback,
        .SensorFailedCallback = SensorFailedCallback
    };

    IoHwAb_Init(&myConfig);  // Khởi tạo Port, ADC, PWM, DIO
    Port_Init(&PortDriverConfig);  
    UART_Display_Init();

    while (1)
    {
        uint16 temperature = IoHwAb_ReadTemperature(0);
        uint16 dutyPercent = 0;
        bool ledState = false;

        // Điều khiển quạt theo nhiệt độ
        if (temperature < 30) {
            dutyPercent = 0;         // Quạt tắt
            ledState = false;
        }
        else if (temperature < 40) {
            dutyPercent = 50;         // Quạt chạy 50%
            ledState = true;
        }
        else {
            dutyPercent = 100;         // Quạt chạy 100%
            ledState = true;
        }

        IoHwAb_SetFanDuty(dutyPercent);
        IoHwAb_SetLed(ledState);

        UART_Display_Send(temperature);
        Delay(1000);
    }
}