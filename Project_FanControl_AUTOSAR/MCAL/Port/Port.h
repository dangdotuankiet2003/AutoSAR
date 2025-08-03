#include "Std_Types.h"
#define PORT_AUTOSAR_VERSION 1  /* Macro để kiểm tra file đúng */

//////////////////// *GENERAL MACROS* ////////////////////
/**< Các Port */
#define PORT_A 0 
#define PORT_B 1 
#define PORT_C 2 
#define PORT_D 3 

/**< Xác định Port từ ID */
#define PORT_GET_PORT(PortId) \
    (((PortId) == PORT_A) ? GPIOA : \
      ((PortId) == PORT_B) ? GPIOB : \
      ((PortId) == PORT_C) ? GPIOC : \
      ((PortId) == PORT_D) ? GPIOD : \
                             NULL_PTR)

/* Định nghĩa các chế độ mode và trạng thái cho Pin */
#define PORT_PIN_MODE_DIO     0
#define PORT_PIN_MODE_ADC     1
#define PORT_PIN_MODE_UART    2
#define PORT_PIN_MODE_PWM     3
#define PORT_PIN_MODE_SPI     4
#define PORT_PIN_MODE_I2C     5
#define PORT_PIN_MODE_CAN     6
#define PORT_PIN_MODE_LIN     7


/**< Mức logic của Port */
#define PORT_PIN_LEVEL_LOW    0
#define PORT_PIN_LEVEL_HIGH   1

/**< Các Macro định nghĩa chân của GPIOx */
#define PORT_PIN_A0  0         
#define PORT_PIN_A1  1
#define PORT_PIN_A2  2
#define PORT_PIN_A3  3
#define PORT_PIN_A4  4
#define PORT_PIN_A5  5
#define PORT_PIN_A6  6
#define PORT_PIN_A7  7
#define PORT_PIN_A8  8
#define PORT_PIN_A9  9
#define PORT_PIN_A10 10
#define PORT_PIN_A11 11
#define PORT_PIN_A12 12
#define PORT_PIN_A13 13
#define PORT_PIN_A14 14
#define PORT_PIN_A15 15

#define PORT_PIN_B0  0
#define PORT_PIN_B1  1
#define PORT_PIN_B2  2
#define PORT_PIN_B3  3
#define PORT_PIN_B4  4
#define PORT_PIN_B5  5
#define PORT_PIN_B6  6
#define PORT_PIN_B7  7
#define PORT_PIN_B8  8
#define PORT_PIN_B9  9
#define PORT_PIN_B10 10
#define PORT_PIN_B11 11
#define PORT_PIN_B12 12
#define PORT_PIN_B13 13
#define PORT_PIN_B14 14
#define PORT_PIN_B15 15

#define PORT_PIN_C0  0
#define PORT_PIN_C1  1
#define PORT_PIN_C2  2
#define PORT_PIN_C3  3
#define PORT_PIN_C4  4
#define PORT_PIN_C5  5
#define PORT_PIN_C6  6
#define PORT_PIN_C7  7
#define PORT_PIN_C8  8
#define PORT_PIN_C9  9
#define PORT_PIN_C10 10
#define PORT_PIN_C11 11
#define PORT_PIN_C12 12
#define PORT_PIN_C13 13
#define PORT_PIN_C14 14
#define PORT_PIN_C15 15

#define PORT_PIN_D0  0
#define PORT_PIN_D1  1
#define PORT_PIN_D2  2
#define PORT_PIN_D3  3
#define PORT_PIN_D4  4
#define PORT_PIN_D5  5
#define PORT_PIN_D6  6
#define PORT_PIN_D7  7
#define PORT_PIN_D8  8
#define PORT_PIN_D9  9
#define PORT_PIN_D10 10
#define PORT_PIN_D11 11
#define PORT_PIN_D12 12
#define PORT_PIN_D13 13
#define PORT_PIN_D14 14
#define PORT_PIN_D15 15

/**< Macro ánh xạ chân */
// 1 << 13 ---> 0001000000000000: bật pin 13 của GPIOx
#define PORT_GET_PIN(PinId) (1U << (PinId))

/**< Macro tạo Port + Pin */
// PortId cần 2 bit -> 2^2 = 4 port
// PinId cần 4 bit -> 2^4 = 16 pin / port => cần dịch trái 4 bit
//=> 8 bit của id = [7:4] của PortID + [3:0] của NumID
// PC13: PORT_PIN(2, 13) = (2 << 4) | 13 = (00000010 -> 00100000) | 00001101 = 0010 1101 = 45.
#define PORT_PIN(PortId, PinId) ((PortId << 4) | (PinId))



//////////////////// *TYPE DEFINITIONS* ////////////////////
/* @typedef: Port_PinType
 * @brief:   Kiểu dữ liệu cho tên ký hiệu cho chân của 1 cổng.
 * @details: Id (0–63) để xác định một chân cụ thể (45 - PC13)
 */
typedef uint8 Port_PinType;

/* @typedef: Port_PinDirectionType
 * @brief:   Kiểu dữ liệu cho hướng cho chân của PORT
 * @details: Xác định chân là input/output 
 */
enum Port_PinDirectionType {
    PORT_PIN_IN, 
    PORT_PIN_OUT
};

/* @typedef: Port_PinModeType
 * @brief:   Kiểu dữ liệu cho chế độ cho chân của PORT
 * @details: Xác định chức năng của chân (GPIO, SPI, I2C, ...)
 */
typedef uint8 Port_PinModeType;

/* @typedef: Port_PinConfigType
 * @brief:   Cấu trúc chứa thông tin cấu hình cho một chân của PORT
 * @details: Lưu các thông tin như ID cổng, số chân, hướng, chế độ, tốc độ, pull, và mức logic ban đầu
 */
typedef struct 
{
    uint8 PortNum;
    Port_PinType PinNum;                   /* 64 Pin */
    Port_PinModeType PinMode;              /* Chế độ: PORT_PIN_MODE_DIO, PORT_PIN_MODE_ADC, ... */
    uint8 PinDirection;                    /* Hướng: PORT_PIN_IN (đầu vào) hoặc PORT_PIN_OUT (đầu ra)*/
    uint8 PinSpeed;                        /* Tốc độ: 0=low, 1=medium, 2=high */
    uint8 PinDirectionChangeable;          /* 0/1: Không/Có thể thay đổi hương */
    uint8 PinLevel;                        /* Mức logic ban đầu: PORT_PIN_LEVEL_HIGH /  PORT_PIN_LEVEL_LOW */
    uint8 PinPull;                         /* 0=none, 1=up, 2=down */
    uint8 ModeChangeable;                  /* 0/1: Không/ Có thể thay đổi chế độ Pin*/
} Port_PinConfigType;
 
/* @typedef: Port_ConfigType
 * @brief:   Thông tin cấu hình cho nhiều chân của PORT
 * @details: Lưu con trỏ tới array lưu cấu hình các chân và số lượng chân, dùng để khởi tạo nhiều chân 
 */
typedef struct {
    const Port_PinConfigType *PinConfigs; 
    uint16 PinCount;                      
} Port_ConfigType;



//////////////////// *FUNCTION DEFINITIONS* ////////////////////
/* @func: Port_Init
 * @brief: Khởi tạo module PORT Driver
 * @param[in] ConfigPtr Con trỏ tới cấu trúc chứa thông tin cấu hình các chân PORT
 * @details: Hàm khởi tạo tất cả các chân PORT theo cấu hình được cung cấp
 */
void Port_Init(const Port_ConfigType* ConfigPtr);

/* @func: Port_SetPinDirection
 * @brief: Đặt hướng (input/output) cho một chân PORT trong runtime
 * @details: Hàm thay đổi hướng của một chân PORT nếu được phép, ví dụ đặt PC13 làm output để điều khiển LED
 */
void Port_SetPinDirection(Port_PinType Pin, enum Port_PinDirectionType Direction);

/* @func: Port_RefreshPortDirection
 * @brief: Làm mới hướng của các chân PORT không được phép thay đổi
 * @details: Hàm khôi phục hướng ban đầu (input/output) của các chân PORT có PinDirectionChangeable = FALSE
 */
void Port_RefreshPortDirection(void);

/* @func: Port_GetVersionInfo
 * @brief: Thông tin phiên bản của module PORT Driver
 * @details: Trả về thông tin phiên bản của PORT Driver
 */
void Port_GetVersionInfo(Std_VersionInfoType* versioninfo);

/* @func: Port_SetPinMode
 * @brief: Đặt chế độ (DIO, ADC, SPI, ...) cho một Pin cuar Port
 * @details: Hàm thiết lập chế độ của một chân PORT 
 */
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode);