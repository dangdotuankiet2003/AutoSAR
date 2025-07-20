#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"

/* Macro ánh xạ cổng cho GPIO */
#define DIO_PORT_A  0   
#define DIO_PORT_B  1   
#define DIO_PORT_C  2   

typedef uint8 ChannelId;

/* Macro xác định cổng GPIO dựa trên Channel Id */
#define DIO_GET_PORT_FR_CHID(ChannelId) \
    (((ChannelId) < 16)  ? GPIOA : \
    ((ChannelId) < 32)  ? GPIOB : \
    ((ChannelId) < 48)  ? GPIOC : \
                         NULL_PTR)

/* Macro xác định cổng GPIO dựa trên Port Id */
#define DIO_GET_PORT_FR_PORTID(PortId) \
    (((PortId) == DIO_PORT_A) ? GPIOA : \
    ((PortId) == DIO_PORT_B) ? GPIOB : \
    ((PortId) == DIO_PORT_C) ? GPIOC : \
                             NULL_PTR)

/* Macro lấy PortNum từ ChannelId */
#define DIO_GET_PORT_NUM(ChannelId) ((ChannelId) >> 4)

/* Macro lấy Pin mask từ ChannelId (1 << PinNum) */
#define DIO_GET_PIN_NUM(ChannelId) (1 << ((ChannelId) & 0x0F))

/* Channel cho tất cả các chân trên GPIOA */
#define DIO_CHANNEL_A0  0  /* Id = 0 */
#define DIO_CHANNEL_A1  1  /* Id = 1 */
#define DIO_CHANNEL_A2  2  /* Id = 2 */
#define DIO_CHANNEL_A3  3  /* Id = 3 */
#define DIO_CHANNEL_A4  4  /* Id = 4 */
#define DIO_CHANNEL_A5  5  /* Id = 5 */
#define DIO_CHANNEL_A6  6  /* Id = 6 */
#define DIO_CHANNEL_A7  7  /* Id = 7 */
#define DIO_CHANNEL_A8  8  /* Id = 8 */
#define DIO_CHANNEL_A9  9  /* Id = 9 */
#define DIO_CHANNEL_A10 10 /* Id = 10 */
#define DIO_CHANNEL_A11 11 /* Id = 11 */
#define DIO_CHANNEL_A12 12 /* Id = 12 */
#define DIO_CHANNEL_A13 13 /* Id = 13 */
#define DIO_CHANNEL_A14 14 /* Id = 14 */
#define DIO_CHANNEL_A15 15 /* Id = 15 */

/* Channel cho tất cả các chân trên GPIOB */
#define DIO_CHANNEL_B0  16  /* Id = 16 */
#define DIO_CHANNEL_B1  17  /* Id = 17 */
#define DIO_CHANNEL_B2  18  /* Id = 18 */
#define DIO_CHANNEL_B3  19  /* Id = 19 */
#define DIO_CHANNEL_B4  20  /* Id = 20 */
#define DIO_CHANNEL_B5  21  /* Id = 21 */
#define DIO_CHANNEL_B6  22  /* Id = 22 */
#define DIO_CHANNEL_B7  23  /* Id = 23 */
#define DIO_CHANNEL_B8  24  /* Id = 24 */
#define DIO_CHANNEL_B9  25  /* Id = 25 */
#define DIO_CHANNEL_B10 26 /* Id = 26 */
#define DIO_CHANNEL_B11 27 /* Id = 27 */
#define DIO_CHANNEL_B12 28 /* Id = 28 */
#define DIO_CHANNEL_B13 29 /* Id = 29 */
#define DIO_CHANNEL_B14 30 /* Id = 30 */
#define DIO_CHANNEL_B15 31 /* Id = 31 */

/* Channel cho tất cả các chân trên GPIOC */
#define DIO_CHANNEL_C0  32  /* Id = 32 */
#define DIO_CHANNEL_C1  33  /* Id = 33 */
#define DIO_CHANNEL_C2  34  /* Id = 34 */
#define DIO_CHANNEL_C3  35  /* Id = 35 */
#define DIO_CHANNEL_C4  36  /* Id = 36 */
#define DIO_CHANNEL_C5  37  /* Id = 37 */
#define DIO_CHANNEL_C6  38  /* Id = 38 */
#define DIO_CHANNEL_C7  39  /* Id = 39 */
#define DIO_CHANNEL_C8  40  /* Id = 40 */
#define DIO_CHANNEL_C9  41  /* Id = 41 */
#define DIO_CHANNEL_C10 42 /* Id = 42 */
#define DIO_CHANNEL_C11 43 /* Id = 43 */
#define DIO_CHANNEL_C12 44 /* Id = 44 */
#define DIO_CHANNEL_C13 45 /* Id = 45 */
#define DIO_CHANNEL_C14 46 /* Id = 46 */
#define DIO_CHANNEL_C15 47 /* Id = 47 */

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