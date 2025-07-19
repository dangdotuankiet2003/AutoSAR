#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "Port.h"
#include "Pwm.h"
#include "Pwm_Cfg.h"
#include "delay.h"
#include "uart_display.h"

// Biến đếm số lần gọi callback
static uint32_t counting = 0;

void Pwm_Channel0_Wrapper(void) {
    counting++; 
    Pwm_Channel0_Notification();  
}

const Port_PinConfigType PortPinsConfig[] = {
    // Pin PA0 cho TIM2_CH1 
    {
        .PortNum = PORT_A,
        .PinNum = 0,
        .PinMode = PORT_PIN_MODE_PWM,
        .PinDirection = PORT_PIN_OUT,
        .PinPull = 0,
        .PinSpeed = 2,
        .PinLevel = PORT_PIN_LEVEL_LOW,
        .PinDirectionChangeable = 0,
        .ModeChangeable = 0
    },
    // Pin PC13 
    {
        .PortNum = PORT_C,
        .PinNum = 13,
        .PinMode = PORT_PIN_MODE_DIO,
        .PinDirection = PORT_PIN_OUT,
        .PinPull = 0,
        .PinSpeed = 2,
        .PinLevel = PORT_PIN_LEVEL_HIGH,
        .PinDirectionChangeable = 0,
        .ModeChangeable = 0
    },
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
    .PinCount = 4,
    .PinConfigs = PortPinsConfig
};

// Define config PWM ở main (1 kênh TIM2_CH1 cho LED test)
// f = f_System / [(Pres + 1) * (ARR + 1)]= 72 MHz / (72 * 100) ≈ 10000 Hz.
const Pwm_ChannelConfigType PwmChannelsConfig[] = {
    {
        .TIMx = TIM2,
        .channel = 1,
        .channelClass = PWM_VARIABLE_PERIOD,
        .polarity = PWM_HIGH, 
        .idleState = PWM_LOW,
        .defaultDutyCycle = 0x0000,
        .defaultPeriod = 100,       /* ARR - Auto Reload Register = 100 */
        .prescaler = 71,            
        .notificationEnable = 1,    /* Bật thông báo */
        .NotificationCb = Pwm_Channel0_Wrapper
    }
};
const Pwm_ConfigType PwmDriverConfig = {
    .numChannels = 1,
    .channels = PwmChannelsConfig
};

int main(void) {
    // System init, clock (72MHz)
    SystemInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    Port_Init(&PortDriverConfig);
    UART_Display_Init();

    Pwm_Init(&PwmDriverConfig);

    // Enable both edges cho kênh 0 (test notification, wrapper tăng counting, gọi PC13)
    Pwm_EnableNotification(0, PWM_BOTH_EDGES);

    while (1) {
        // Sáng dần: Tăng duty từ 0% đến 100%
        for (uint16_t duty = 0x0000; duty <= 0x8000; duty += 0x0080) {
            Pwm_SetDutyCycle(0, duty);
            UART_Display_Send(counting);  
            Delay(5);
        }

        // Tối dần: Giảm duty từ 100% về 0%
        for (uint16_t duty = 0x8000; duty >= 0x0000; duty -= 0x0080) {
            Pwm_SetDutyCycle(0, duty);
            UART_Display_Send(counting);  
            Delay(5);
        }
        Delay(500);
    }
}