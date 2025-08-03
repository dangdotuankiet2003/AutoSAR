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
#define DIO_GET_PIN_MASK(ChannelId) (1 << ((ChannelId) & 0x0F))

/* Macro lấy số pin từ ChannelId (0-15) */
#define DIO_GET_PIN_NUMBER(ChannelId) ((ChannelId) & 0x0F)

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
 * @example: DIO_CHANNEL_C13 ((Dio_ChannelType)45) = 45
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
    uint8 offset;      /**< Độ dịch của bit đầu tiên trong nhóm */
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
 * @brief: Kiểu dữ liệu cho mức logic (0/1) của cổng DIO với 16 kênh
 * @details: Đây là kiểu biểu thị trạng thái của tất cả kênh trong 1 cổng
 * @example: Dio_PortLevelType ledValue = (buttonState == STD_HIGH) ? 0xFF : 0x00; 
 */
typedef uint16 Dio_PortLevelType;

///////////////// FUNCTION DEFINITIONS ////////////////////

/**
 * @brief: Hàm đọc mức logic của 1 kênh DIO
 * @param[in] ChannelId: ID của kênh DIO cần đọc
 * @return: Dio_LevelType: Mức logic của kênh (STD_HIGH hoặc STD_LOW)
 * @details: Đọc trạng thái input của chân GPIO tương ứng với ChannelId
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @brief: Hàm ghi mức logic cho 1 kênh DIO
 * @param[in] ChannelId: ID của kênh DIO cần ghi
 * @param[in] Level: Mức logic cần ghi (STD_HIGH hoặc STD_LOW)
 * @details: Đặt trạng thái output của chân GPIO tương ứng
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief: Hàm đọc trạng thái của 1 cổng DIO
 * @param[in] PortId: ID của cổng DIO cần đọc
 * @return: Dio_PortLevelType: Trạng thái của cổng (bitmask 16 bit)
 * @details: Đọc toàn bộ thanh ghi input của cổng GPIO
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @brief: Hàm ghi trạng thái cho 1 cổng DIO
 * @param[in] PortId: ID của cổng DIO cần ghi
 * @param[in] Level: Giá trị bitmask cần ghi (16 bit)
 * @details: Ghi toàn bộ thanh ghi output của cổng GPIO
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @brief: Hàm đọc trạng thái của nhóm kênh DIO
 * @param[in] ChannelGroupIdPtr: Con trỏ đến cấu trúc nhóm kênh
 * @return: Dio_PortLevelType: Trạng thái của nhóm kênh (bitmask dịch offset)
 * @details: Đọc và mask giá trị từ cổng GPIO theo nhóm
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr);

/**
 * @brief: Hàm ghi trạng thái cho nhóm kênh DIO
 * @param[in] ChannelGroupIdPtr: Con trỏ đến cấu trúc nhóm kênh
 * @param[in] Level: Giá trị bitmask cần ghi cho nhóm
 * @details: Mask và ghi giá trị vào nhóm kênh trong cổng GPIO
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @brief: Hàm lấy thông tin phiên bản của module DIO
 * @param[out] VersionInfo: Con trỏ đến cấu trúc lưu thông tin phiên bản
 * @details: Ghi vendorID, moduleID, và phiên bản phần mềm vào cấu trúc
 */
void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**
 * @brief: Hàm đảo trạng thái logic của 1 kênh DIO
 * @param[in] ChannelId: ID của kênh DIO cần đảo
 * @return: Dio_LevelType: Trạng thái sau khi đảo (STD_HIGH hoặc STD_LOW)
 * @details: Đọc trạng thái hiện tại và đảo (HIGH -> LOW hoặc ngược lại)
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId);

/**
 * @brief: Hàm ghi trạng thái cho 1 số kênh đầu ra được chọn trong 1 cổng DIO
 * @param[in] PortId: ID của cổng DIO cần ghi
 * @param[in] Level: Giá trị bitmask cần ghi cho các kênh được chọn
 * @param[in] Mask: Bitmask chọn các kênh để ghi
 * @details: Mask và ghi giá trị vào cổng GPIO
 */
void Dio_MaskedWritePort(Dio_PortType PortId, Dio_PortLevelType Level, Dio_PortLevelType Mask);

#endif /* DIO_H */