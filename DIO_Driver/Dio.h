#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"

/* Macro ánh xạ cổng cho GPIO */
#define DIO_PORT_A  0   
#define DIO_PORT_B  1   
#define DIO_PORT_C  2   
#define DIO_PORT_D  3   

typedef uint8 ChannelId;

/* Macro xác định cổng GPIO dựa trên Channel Id */
#define DIO_GET_PORT_FR_CHID(ChannelId) \
    (((ChannelId) < 16)  ? GPIOA : \
    ((ChannelId) < 32)  ? GPIOB : \
    ((ChannelId) < 48)  ? GPIOC : \
    ((ChannelId) < 64)  ? GPIOD : \
                         NULL_PTR)

/* Macro xác định cổng GPIO dựa trên Port Id */
#define DIO_GET_PORT_FR_PORTID(PortId) \
    (((PortId) == DIO_PORT_A) ? GPIOA : \
    ((PortId) == DIO_PORT_B) ? GPIOB : \
    ((PortId) == DIO_PORT_B) ? GPIOC : \
    ((PortId) == DIO_PORT_D) ? GPIOD : \
                             NULL_PTR)

/* Macro xác định chân GPIO dựa trên Channel Id */
#define DIO_GET_PIN(ChannelId) (1 << (ChannelId % 16))

/* Macro tạo Channel Id từ GPIOx và Pin */
// PB4: 1 << 4 = 00010000 = 16 => Id = 16 + 4 = 20
#define DIO_CHANNEL(GPIOx, Pin) (((GPIOx) << 4) + (Pin))

/* Channel cho tất cả các chân trên GPIOA */
#define DIO_CHANNEL_A0  DIO_CHANNEL(DIO_PORT_A, 0)  /* GPIOA Pin 0 */
#define DIO_CHANNEL_A1  DIO_CHANNEL(DIO_PORT_A, 1)  /* GPIOA Pin 1 */
#define DIO_CHANNEL_A2  DIO_CHANNEL(DIO_PORT_A, 2)  /* GPIOA Pin 2 */
#define DIO_CHANNEL_A3  DIO_CHANNEL(DIO_PORT_A, 3)  /* GPIOA Pin 3 */
#define DIO_CHANNEL_A4  DIO_CHANNEL(DIO_PORT_A, 4)  /* GPIOA Pin 4 */
#define DIO_CHANNEL_A5  DIO_CHANNEL(DIO_PORT_A, 5)  /* GPIOA Pin 5 */
#define DIO_CHANNEL_A6  DIO_CHANNEL(DIO_PORT_A, 6)  /* GPIOA Pin 6 */
#define DIO_CHANNEL_A7  DIO_CHANNEL(DIO_PORT_A, 7)  /* GPIOA Pin 7 */
#define DIO_CHANNEL_A8  DIO_CHANNEL(DIO_PORT_A, 8)  /* GPIOA Pin 8 */
#define DIO_CHANNEL_A9  DIO_CHANNEL(DIO_PORT_A, 9)  /* GPIOA Pin 9 */
#define DIO_CHANNEL_A10 DIO_CHANNEL(DIO_PORT_A, 10) /* GPIOA Pin 10 */
#define DIO_CHANNEL_A11 DIO_CHANNEL(DIO_PORT_A, 11) /* GPIOA Pin 11 */
#define DIO_CHANNEL_A12 DIO_CHANNEL(DIO_PORT_A, 12) /* GPIOA Pin 12 */
#define DIO_CHANNEL_A13 DIO_CHANNEL(DIO_PORT_A, 13) /* GPIOA Pin 13 */
#define DIO_CHANNEL_A14 DIO_CHANNEL(DIO_PORT_A, 14) /* GPIOA Pin 14 */
#define DIO_CHANNEL_A15 DIO_CHANNEL(DIO_PORT_A, 15) /* GPIOA Pin 15 */

/* Channel cho tất cả các chân trên GPIOB */
#define DIO_CHANNEL_B0  DIO_CHANNEL(DIO_PORT_B, 0)  /* GPIOB Pin 0 */
#define DIO_CHANNEL_B1  DIO_CHANNEL(DIO_PORT_B, 1)  /* GPIOB Pin 1 */
#define DIO_CHANNEL_B2  DIO_CHANNEL(DIO_PORT_B, 2)  /* GPIOB Pin 2 */
#define DIO_CHANNEL_B3  DIO_CHANNEL(DIO_PORT_B, 3)  /* GPIOB Pin 3 */
#define DIO_CHANNEL_B4  DIO_CHANNEL(DIO_PORT_B, 4)  /* GPIOB Pin 4 */
#define DIO_CHANNEL_B5  DIO_CHANNEL(DIO_PORT_B, 5)  /* GPIOB Pin 5 */
#define DIO_CHANNEL_B6  DIO_CHANNEL(DIO_PORT_B, 6)  /* GPIOB Pin 6 */
#define DIO_CHANNEL_B7  DIO_CHANNEL(DIO_PORT_B, 7)  /* GPIOB Pin 7 */
#define DIO_CHANNEL_B8  DIO_CHANNEL(DIO_PORT_B, 8)  /* GPIOB Pin 8 */
#define DIO_CHANNEL_B9  DIO_CHANNEL(DIO_PORT_B, 9)  /* GPIOB Pin 9 */
#define DIO_CHANNEL_B10 DIO_CHANNEL(DIO_PORT_B, 10) /* GPIOB Pin 10 */
#define DIO_CHANNEL_B11 DIO_CHANNEL(DIO_PORT_B, 11) /* GPIOB Pin 11 */
#define DIO_CHANNEL_B12 DIO_CHANNEL(DIO_PORT_B, 12) /* GPIOB Pin 12 */
#define DIO_CHANNEL_B13 DIO_CHANNEL(DIO_PORT_B, 13) /* GPIOB Pin 13 */
#define DIO_CHANNEL_B14 DIO_CHANNEL(DIO_PORT_B, 14) /* GPIOB Pin 14 */
#define DIO_CHANNEL_B15 DIO_CHANNEL(DIO_PORT_B, 15) /* GPIOB Pin 15 */

/* Channel cho tất cả các chân trên GPIOC */
#define DIO_CHANNEL_C0  DIO_CHANNEL(DIO_PORT_C, 0)  /* GPIOC Pin 0 */
#define DIO_CHANNEL_C1  DIO_CHANNEL(DIO_PORT_C, 1)  /* GPIOC Pin 1 */
#define DIO_CHANNEL_C2  DIO_CHANNEL(DIO_PORT_C, 2)  /* GPIOC Pin 2 */
#define DIO_CHANNEL_C3  DIO_CHANNEL(DIO_PORT_C, 3)  /* GPIOC Pin 3 */
#define DIO_CHANNEL_C4  DIO_CHANNEL(DIO_PORT_C, 4)  /* GPIOC Pin 4 */
#define DIO_CHANNEL_C5  DIO_CHANNEL(DIO_PORT_C, 5)  /* GPIOC Pin 5 */
#define DIO_CHANNEL_C6  DIO_CHANNEL(DIO_PORT_C, 6)  /* GPIOC Pin 6 */
#define DIO_CHANNEL_C7  DIO_CHANNEL(DIO_PORT_C, 7)  /* GPIOC Pin 7 */
#define DIO_CHANNEL_C8  DIO_CHANNEL(DIO_PORT_C, 8)  /* GPIOC Pin 8 */
#define DIO_CHANNEL_C9  DIO_CHANNEL(DIO_PORT_C, 9)  /* GPIOC Pin 9 */
#define DIO_CHANNEL_C10 DIO_CHANNEL(DIO_PORT_C, 10) /* GPIOC Pin 10 */
#define DIO_CHANNEL_C11 DIO_CHANNEL(DIO_PORT_C, 11) /* GPIOC Pin 11 */
#define DIO_CHANNEL_C12 DIO_CHANNEL(DIO_PORT_C, 12) /* GPIOC Pin 12 */
#define DIO_CHANNEL_C13 DIO_CHANNEL(DIO_PORT_C, 13) /* GPIOC Pin 13 */
#define DIO_CHANNEL_C14 DIO_CHANNEL(DIO_PORT_C, 14) /* GPIOC Pin 14 */
#define DIO_CHANNEL_C15 DIO_CHANNEL(DIO_PORT_C, 15) /* GPIOC Pin 15 */

/* Channel cho tất cả các chân trên GPIOD */
#define DIO_CHANNEL_D0  DIO_CHANNEL(DIO_PORT_D, 0)  /* GPIOD Pin 0 */
#define DIO_CHANNEL_D1  DIO_CHANNEL(DIO_PORT_D, 1)  /* GPIOD Pin 1 */
#define DIO_CHANNEL_D2  DIO_CHANNEL(DIO_PORT_D, 2)  /* GPIOD Pin 2 */
#define DIO_CHANNEL_D3  DIO_CHANNEL(DIO_PORT_D, 3)  /* GPIOD Pin 3 */
#define DIO_CHANNEL_D4  DIO_CHANNEL(DIO_PORT_D, 4)  /* GPIOD Pin 4 */
#define DIO_CHANNEL_D5  DIO_CHANNEL(DIO_PORT_D, 5)  /* GPIOD Pin 5 */
#define DIO_CHANNEL_D6  DIO_CHANNEL(DIO_PORT_D, 6)  /* GPIOD Pin 6 */
#define DIO_CHANNEL_D7  DIO_CHANNEL(DIO_PORT_D, 7)  /* GPIOD Pin 7 */
#define DIO_CHANNEL_D8  DIO_CHANNEL(DIO_PORT_D, 8)  /* GPIOD Pin 8 */
#define DIO_CHANNEL_D9  DIO_CHANNEL(DIO_PORT_D, 9)  /* GPIOD Pin 9 */
#define DIO_CHANNEL_D10 DIO_CHANNEL(DIO_PORT_D, 10) /* GPIOD Pin 10 */
#define DIO_CHANNEL_D11 DIO_CHANNEL(DIO_PORT_D, 11) /* GPIOD Pin 11 */
#define DIO_CHANNEL_D12 DIO_CHANNEL(DIO_PORT_D, 12) /* GPIOD Pin 12 */
#define DIO_CHANNEL_D13 DIO_CHANNEL(DIO_PORT_D, 13) /* GPIOD Pin 13 */
#define DIO_CHANNEL_D14 DIO_CHANNEL(DIO_PORT_D, 14) /* GPIOD Pin 14 */
#define DIO_CHANNEL_D15 DIO_CHANNEL(DIO_PORT_D, 15) /* GPIOD Pin 15 */

///////////////// TYPE DEFINITIONS ////////////////////

/**
 * @typedef: Dio_ChannelType
 * @brief:   Kiểu dữ liệu cho ID của kênh DIO
 * @details: Đây là ID số cho 1 kênh DIO cụ thể
 * @example: DIO_CHANNEL_BUTTON_PC4 ((Dio_ChannelType)0x04)
 */
typedef uint8 Dio_ChannelType;

/**
 * @typedef: Dio_PortType
 * @brief: Kiểu dữ liệu cho 1 DIO Port
 * @details: Đây là kiểu định danh (ID) cho 1 port cụ thể
 * @example: #define DIO_PORT_C ((Dio_PortType)0x02)
 */
typedef uint8 Dio_PortType;

/**
 * @typedef: Dio_ChannelGroupType
 * @brief: Kiểu dữ liệu cho nhóm kênh DIO
 * @details: Đây là kiểu định danh cho nhiều kênh liền kề trong 1 cổng
 * @example: 
 * #define DIO_CHANNEL_GROUP_SENSOR_PC0_PC3 { \
 * .mask = 0x0F, 
 *  .offset = 0,  
 *  .port = DIO_PORT_C }
 */

typedef struct 
{
    uint16 mask;      
    uint8 offset;      /**<Độ dịch của bit đầu tiên trong nhóm */
    Dio_PortType port; 
}Dio_ChannelGroupType;

/**
 * @typedef: Dio_LevelType
 * @brief: Kiểu dữ liệu cho mức logic của kênh DIO
 * @details: Đây là kiểu xác định trạng thái logic (cao/thấp) của 1 kênh DIO
 * @example: Dio_LevelType ledState = STD_HIGH;
 */
typedef uint8 Dio_LevelType;

/**
 * @typedef: Dio_PortLevelType
 * @brief: Kiểu dữ liệu cho mức logic(0/1) của cổng DIO vs 16 kênh
 * @details: Đây là kiểu biểu thị trạng thái của tất cả kênh trong 1 cổng
 * @example: Dio_PortLevelType ledValue = (buttonState == STD_HIGH) ? 0xFF : 0x00; 
 */
typedef uint16 Dio_PortLevelType;



///////////////// FUNCTION DEFINITIONS ////////////////////

/**
 * @typedef: Dio_ReadChannel
 * @brief: Hàm đọc mức logic của 1 kênh DIO
 * @details: Trả về trạng thái logic (STD_HIGH/STD_LOW) của kênh được chỉ định
 * @return Dio_LevelType: Mức logic của kênh
 * @example: 
 *   Dio_ChannelType buttonChannel = DIO_CHANNEL_BUTTON_PC4; // ID cho PC4
 *   Dio_LevelType buttonState = Dio_ReadChannel(buttonChannel); // Đọc trạng thái nút bấm
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @void: Dio_WriteChannel
 * @brief: Hàm ghi mức logic cho 1 kênh DIO
 * @details: Đặt trạng thái logic (STD_HIGH/STD_LOW) cho kênh được chỉ định
 * @param ChannelId: ID của kênh DIO cần ghi
 * @example:
 *   #define DIO_CHANNEL_LED_PB2 (Dio_ChannelType(0x02))
 *   Dio_ChannelType ledChannel = DIO_CHANNEL_LED_PB2; 
 *   Dio_WriteChannel(ledChannel, STD_HIGH);
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @typedef: Dio_ReadPort
 * @brief: Hàm đọc trạng thái của 1 cổng DIO
 * @details: Trả về trạng thái logic (bitmask) của tất cả kênh trong cổng
 * @param PortId: ID của cổng DIO cần đọc
 * @return Dio_PortLevelType: Trạng thái của cổng
 * @example:
 *   Dio_PortType buttonPort = DIO_PORT_C;
 *   Dio_PortLevelType portState = Dio_ReadPort(buttonPort); 
 *   if (portState & 0x10){}
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @typedef: Dio_WritePort
 * @brief: Hàm ghi trạng thái cho 1 cổng DIO
 * @details: Đặt trạng thái logic (bitmask) cho tất cả kênh đầu ra trong cổng
 * @example:
 *   Dio_PortType ledPort = DIO_PORT_B; 
 *   Dio_PortLevelType ledValue = 0xFF; 
 *   Dio_WritePort(ledPort, ledValue); 
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @typedef: Dio_ReadChannelGroup
 * @brief: Hàm đọc trạng thái của nhóm kênh DIO
 * @details: Trả về trạng thái logic (bitmask) của các kênh liền kề trong nhóm
 * @param ChannelGroupIdPtr: Con trỏ đến cấu trúc nhóm kênh
 * @return Dio_PortLevelType: Trạng thái của nhóm kênh
 * @example:
 *   const Dio_ChannelGroupType sensorGroup = DIO_CHANNEL_GROUP_SENSOR_PC0_PC3; 
 *   Dio_PortLevelType groupState = Dio_ReadChannelGroup(&sensorGroup); 
 *   => 0x0101: PC0 & PC2 High, PC1 & PC3 Low
 */
Dio_PortLevelType Dio_ReadChannelGroup (const Dio_ChannelGroupType* ChannelGroupIdPtr);

/**
 * @typedef: Dio_WriteChannelGroup
 * @brief: Hàm ghi trạng thái cho nhóm kênh DIO
 * @details: Đặt trạng thái logic (bitmask) cho các kênh đầu ra liền kề trong nhóm
 * @param ChannelGroupIdPtr: Con trỏ đến cấu trúc nhóm kênh
 * @param Level: Giá trị bitmask cần ghi
 * @example:
 *   const Dio_ChannelGroupType ledGroup = DIO_CHANNEL_GROUP_SENSOR_PA0_PA3; 
 *   Dio_WriteChannelGroup(&ledGroup, 0x05); 
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @typedef: Dio_GetVersionInfo
 * @brief: Hàm lấy thông tin phiên bản của module DIO
 * @details: Ghi thông tin phiên bản (vendorID, moduleID, phiên bản phần mềm) vào con trỏ
 */
void Dio_GetVersionInfo (Std_VersionInfoType* VersionInfo);

/**
 * @typedef: Dio_FlipChannel
 * @brief: Hàm đảo trạng thái logic của 1 kênh DIO
 * @details: Đổi trạng thái kênh (HIGH thành LOW hoặc ngược lại) và trả về trạng thái sau đảo
 */
Dio_LevelType Dio_FlipChannel (Dio_ChannelType ChannelId);

/**
 * @typedef: Dio_MaskedWritePort
 * @brief: Hàm ghi trạng thái cho 1 số kênh đầu ra được chọn trong 1 cổng DIO
 * @details: Ghi trạng thái logic (bitmask) cho các kênh đầu ra được chọn bởi mask
 * @param Level: Giá trị 0/1 cho các bitmask của các kênh cần ghi 
 * @param Mask: Bitmask chọn các kênh để ghi giá trị
 * @example:
 *   Dio_PortType ledPort = DIO_PORT_B; 
 *   Dio_MaskedWritePort(ledPort, 0x05, 0x0F); //PA0,2 HIGH ; PA1,3 LOW
 */
void Dio_MaskedWritePort (Dio_PortType PortId,Dio_PortLevelType Level, Dio_PortLevelType Mask);

#endif 