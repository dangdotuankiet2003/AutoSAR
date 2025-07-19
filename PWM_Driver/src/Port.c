#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "Port.h"

#ifndef PORT_AUTOSAR_VERSION
#error "Wrong Port.h included"
#endif

// Lưu cấu hình từ Port_Init để sử dụng trong các hàm khác
static const Port_ConfigType *Port_Config = NULL_PTR;

// Hàm cấu hình một chân GPIO
void Port_ConfigurePin(uint8 PortNum, Port_PinType PinNum, Port_PinModeType PinMode,
                       uint8 Direction, uint8 PinPull, uint8 PinSpeed)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // Kiểm tra tham số nhập vào có hợp lệ không
    if(Direction > 1 || PinPull > 2 || PinSpeed > 2) return;

    // Bật xung Clock
    switch(PortNum){
        case PORT_A: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); break;
        case PORT_B: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); break;
        case PORT_C: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); break;
        case PORT_D: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); break;
        default: return;      
    }
    // Bật clock cho AFIO (cần cho I2C, PWM, UART,...)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // Xác định cổng GPIO từ PortNum
    GPIO_TypeDef *GPIOx = PORT_GET_PORT(PortNum);
    if(GPIOx == NULL_PTR) return;

    // Xác định chân của Port từ PinNum
    uint16 gpioPin = PORT_GET_PIN(PinNum);
    if(gpioPin == 0) return;

    GPIO_InitStructure.GPIO_Pin = gpioPin;
    // Xác định MODE hoạt động cho Pin của Port
    switch(PinMode){
        // DIO MODE
        case PORT_PIN_MODE_DIO:
            if(Direction == PORT_PIN_OUT){
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            }
            else{
                if(PinPull == 1) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       // Input - Pullup
                else if(PinPull == 2) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input - Pulldown
                else GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            }
            break;
        
        // UART MODE
        case PORT_PIN_MODE_UART:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            break;
        
        // ADC MODE
        case PORT_PIN_MODE_ADC:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            break;
        
        // I2C MODE
        case PORT_PIN_MODE_I2C:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // Alternate function open-drain cho I2C
            break;
        
        // PWM/TIM MODE 
        case PORT_PIN_MODE_PWM:
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  /* Alternate Function Push-Pull cho PWM output từ timer */
            break;
        
        default: return; 
    }

    // Xác định SPEED hoạt động cho Pin của Port
    switch(PinSpeed){
        case 0: // Low
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
            break;
        case 1: // Medium
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
            break;
        case 2: // High
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            break;
        default: GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; break;
    }
    // Khởi tạo GPIO cho Port
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

// [SWS_Port_00041]
void Port_Init(const Port_ConfigType* ConfigPtr){
    if(ConfigPtr == NULL_PTR) return;

    // Lưu cấu hình để sử dụng trong các hàm khác
    Port_Config = ConfigPtr;

    for(uint8 i = 0; i < ConfigPtr->PinCount; i++){
        /* Kiểm tra Direction hợp lệ */
        if (ConfigPtr->PinConfigs[i].PinDirection > 1) continue;

        // Lấy thông tin chân
        GPIO_TypeDef *GPIOx = PORT_GET_PORT(ConfigPtr->PinConfigs[i].PortNum);
        if(GPIOx == NULL_PTR) continue;
        uint16 gpioPin = PORT_GET_PIN(ConfigPtr->PinConfigs[i].PinNum);
        if(gpioPin == 0) continue;

        Port_ConfigurePin(
            ConfigPtr->PinConfigs[i].PortNum,
            ConfigPtr->PinConfigs[i].PinNum,
            ConfigPtr->PinConfigs[i].PinMode,
            ConfigPtr->PinConfigs[i].PinDirection,
            ConfigPtr->PinConfigs[i].PinPull,
            ConfigPtr->PinConfigs[i].PinSpeed
        );

        // Khởi tạo logic ban đầu nếu Pin là Output
        if(ConfigPtr->PinConfigs[i].PinDirection == PORT_PIN_OUT){
            if(ConfigPtr->PinConfigs[i].PinLevel == PORT_PIN_LEVEL_LOW) GPIO_ResetBits(GPIOx, gpioPin);                     //LOW
            else if(ConfigPtr->PinConfigs[i].PinLevel == PORT_PIN_LEVEL_HIGH) GPIO_SetBits(GPIOx, gpioPin);                 //HIGH
        }
    }
}

void Port_SetPinDirection(Port_PinType Pin, enum Port_PinDirectionType Direction)
{
    /* Kiểm tra cấu hình và hướng hợp lệ */
    if (Port_Config == NULL_PTR || Direction > PORT_PIN_OUT) return;

    for (uint8 i = 0; i < Port_Config->PinCount; i++)
    {
        /* Lấy thông tin chân */
        GPIO_TypeDef* GPIOx = PORT_GET_PORT(Port_Config->PinConfigs[i].PortNum);
        if (GPIOx == NULL_PTR) continue; 
            
        uint16 gpioPin = PORT_GET_PIN(Port_Config->PinConfigs[i].PinNum);
        if (gpioPin == 0) continue; 
            
        /* Kiểm tra ID của Pin */
        if (Port_Config->PinConfigs[i].PortNum == (Pin >> 4) && Port_Config->PinConfigs[i].PinNum == (Pin & 0x0F))
        {
            /* Kiểm tra hướng hiện tại hợp lệ */
            if (Port_Config->PinConfigs[i].PinDirection > 1) return;

            /* Kiểm tra quyền thay đổi hướng */
            if (Port_Config->PinConfigs[i].PinDirectionChangeable == 0) return; 
               
            /* Cấu hình Pin với hướng mới */
            Port_ConfigurePin(
                Port_Config->PinConfigs[i].PortNum,
                Port_Config->PinConfigs[i].PinNum,
                Port_Config->PinConfigs[i].PinMode,
                (uint8)Direction,
                Port_Config->PinConfigs[i].PinPull,
                Port_Config->PinConfigs[i].PinSpeed
            );
            return; 
        }
    }
}

void Port_RefreshPortDirection(void){
    /* Kiểm tra cấu hình hợp lệ */
    if (Port_Config == NULL_PTR) return;

    for(uint8 i = 0; i < Port_Config->PinCount; i++){
        GPIO_TypeDef *GPIOx = PORT_GET_PORT(Port_Config->PinConfigs[i].PortNum);
        if(GPIOx == NULL_PTR) continue;
        uint16 gpioPin = PORT_GET_PIN(Port_Config->PinConfigs[i].PinNum);
        if(gpioPin == 0) continue;

        // Kiểm tra hướng nếu hợp lệ
        if(Port_Config->PinConfigs[i].PinDirection > 1) continue;
        
        // Khôi phục về mặc định
        Port_ConfigurePin(
            Port_Config->PinConfigs[i].PortNum,
            Port_Config->PinConfigs[i].PinNum,
            Port_Config->PinConfigs[i].PinMode,
            Port_Config->PinConfigs[i].PinDirection,
            Port_Config->PinConfigs[i].PinPull,
            Port_Config->PinConfigs[i].PinSpeed
        );
        // Tạo logic ban đầu nếu Pin là Output
        if(Port_Config->PinConfigs[i].PinDirection == PORT_PIN_OUT){
            if(Port_Config->PinConfigs[i].PinLevel == PORT_PIN_LEVEL_LOW) GPIO_ResetBits(GPIOx, gpioPin);             //LOW
            else if(Port_Config->PinConfigs[i].PinLevel == PORT_PIN_LEVEL_HIGH) GPIO_SetBits(GPIOx, gpioPin);         //HIGH
        }
    }
}

void Port_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    if (versioninfo == NULL_PTR) return;

    versioninfo->vendorID = 0x01; 
    versioninfo->moduleID = 0x02; 
    versioninfo->sw_major_version = 1; 
    versioninfo->sw_minor_version = 0; 
    versioninfo->sw_patch_version = 0; 
}

void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode){
    /* Kiểm tra cấu hình hợp lệ */
    if (Port_Config == NULL_PTR) return;

    for(uint8 i = 0; i < Port_Config->PinCount; i++){
        // Kiểm tra Id Pin có hợp lệ không
        if((Port_Config->PinConfigs[i].PortNum == (Pin>>4)) && (Port_Config->PinConfigs[i].PinNum == (Pin & 0x0F))){
            // Kiểm tra xem có bật ModeChangeable không
            if(Port_Config->PinConfigs[i].ModeChangeable == 0) return;
            
            Port_ConfigurePin(
                Port_Config->PinConfigs[i].PortNum,
                Port_Config->PinConfigs[i].PinNum,
                Mode,
                Port_Config->PinConfigs[i].PinDirection,
                Port_Config->PinConfigs[i].PinPull,
                Port_Config->PinConfigs[i].PinSpeed
            );
            return;
        }
    }
}