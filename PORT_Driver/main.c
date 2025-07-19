#include "stm32f10x.h"
#include "Dio.h"
#include "Port.h"
#include "delay.h"

#ifndef PORT_AUTOSAR_VERSION
#error "Wrong Port.h included"
#endif

int main(void) {
    // PC13: PORT_PIN(2, 13) = (2 << 4) | 13 = 0010 1101 = 45
    Port_PinType pinPC13 = PORT_PIN(PORT_C, PORT_PIN_C13);

    // Cấu hình PC13 làm GPIO output
    Port_PinConfigType pinConfigPC13 = {
        .PortNum = PORT_C,                    
        .PinNum = PORT_PIN_C13,               
        .PinMode = PORT_PIN_MODE_DIO,        
        .PinDirection = PORT_PIN_OUT,        
        .PinSpeed = 2,                        
        .PinDirectionChangeable = 1,          
        .PinLevel = PORT_PIN_LEVEL_LOW,       
        .PinPull = 0,                         
        .ModeChangeable = 0                   
    };

    Port_ConfigType portConfig = {
        .PinConfigs = &pinConfigPC13,         // Con trỏ đến cấu hình PC13
        .PinCount = 1                         
    };

    // Khởi tạo PORT
    Port_Init(&portConfig);
    Port_SetPinDirection(pinPC13, PORT_PIN_OUT);


    while (1) {
        Dio_FlipChannel(DIO_CHANNEL_C13); 
        Delay_ms(500);                   
    }
}