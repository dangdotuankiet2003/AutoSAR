#include "Dio.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

/**
 * @brief: Hàm đọc mức logic của 1 kênh DIO
 * @param[in] ChannelId: ID của kênh DIO cần đọc
 * @return: Dio_LevelType: Mức logic của kênh (STD_HIGH hoặc STD_LOW)
 * @details: Đọc trạng thái input của chân GPIO tương ứng với ChannelId
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_CHID(ChannelId);
    uint16 Pin = DIO_GET_PIN_MASK(ChannelId);  /* Sử dụng DIO_GET_PIN_MASK để lấy bit mask pin */

    if(Port == NULL_PTR){
        return STD_LOW;
    }
    
    return (GPIO_ReadInputDataBit(Port, Pin) == Bit_SET) ? STD_HIGH : STD_LOW;
}

/**
 * @brief: Hàm ghi mức logic cho 1 kênh DIO
 * @param[in] ChannelId: ID của kênh DIO cần ghi
 * @param[in] Level: Mức logic cần ghi (STD_HIGH hoặc STD_LOW)
 * @details: Đặt trạng thái output của chân GPIO tương ứng
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level){
   GPIO_TypeDef *Port = DIO_GET_PORT_FR_CHID(ChannelId);
   uint16 Pin = DIO_GET_PIN_MASK(ChannelId);  /* Sử dụng DIO_GET_PIN_MASK để lấy bit mask pin */
  
    if(Port == NULL_PTR){
        return;
    }

    if(Level == STD_HIGH) GPIO_SetBits(Port, Pin);
    else GPIO_ResetBits(Port, Pin);
}

/**
 * @brief: Hàm đọc trạng thái của 1 cổng DIO
 * @param[in] PortId: ID của cổng DIO cần đọc
 * @return: Dio_PortLevelType: Trạng thái của cổng (bitmask 16 bit)
 * @details: Đọc toàn bộ thanh ghi input của cổng GPIO
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(PortId);
    if(Port == NULL_PTR) return 0;
    return (Dio_PortLevelType)(GPIO_ReadInputData(Port));
}

/**
 * @brief: Hàm ghi trạng thái cho 1 cổng DIO
 * @param[in] PortId: ID của cổng DIO cần ghi
 * @param[in] Level: Giá trị bitmask cần ghi (16 bit)
 * @details: Ghi toàn bộ thanh ghi output của cổng GPIO
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(PortId);
    if(Port == NULL_PTR) return;
    GPIO_Write(Port, Level);
}

/**
 * @brief: Hàm đọc trạng thái của nhóm kênh DIO
 * @param[in] ChannelGroupIdPtr: Con trỏ đến cấu trúc nhóm kênh
 * @return: Dio_PortLevelType: Trạng thái của nhóm kênh (bitmask dịch offset)
 * @details: Đọc và mask giá trị từ cổng GPIO theo nhóm
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr){
   GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(ChannelGroupIdPtr->port);
   if(Port == NULL_PTR) return 0;
   uint16 portValue = GPIO_ReadInputData(Port);

   return (Dio_PortLevelType)((portValue & ChannelGroupIdPtr->mask) >> ChannelGroupIdPtr->offset);
}

/**
 * @brief: Hàm ghi trạng thái cho nhóm kênh DIO
 * @param[in] ChannelGroupIdPtr: Con trỏ đến cấu trúc nhóm kênh
 * @param[in] Level: Giá trị bitmask cần ghi cho nhóm
 * @details: Mask và ghi giá trị vào nhóm kênh trong cổng GPIO
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(ChannelGroupIdPtr->port);
    if(Port == NULL_PTR) return;
    
    uint16_t portValue = GPIO_ReadInputData(Port);
    portValue &= ~(ChannelGroupIdPtr->mask); /* Reset tương ứng với các bit mask */
    portValue |= ((Level << ChannelGroupIdPtr->offset) & (ChannelGroupIdPtr->mask));
    
    GPIO_Write(Port, portValue);
}  

/**
 * @brief: Hàm lấy thông tin phiên bản của module DIO
 * @param[out] VersionInfo: Con trỏ đến cấu trúc lưu thông tin phiên bản
 * @details: Ghi vendorID, moduleID, và phiên bản phần mềm vào cấu trúc
 */
void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo){
    if (VersionInfo == NULL_PTR) return;

    VersionInfo->moduleID = 0x0001;
    VersionInfo->vendorID = 0x0002;
    VersionInfo->sw_major_version = 1;
    VersionInfo->sw_minor_version = 0;
    VersionInfo->sw_patch_version = 0;
}

/**
 * @brief: Hàm đảo trạng thái logic của 1 kênh DIO
 * @param[in] ChannelId: ID của kênh DIO cần đảo
 * @return: Dio_LevelType: Trạng thái sau khi đảo (STD_HIGH hoặc STD_LOW)
 * @details: Đọc trạng thái hiện tại và đảo (HIGH -> LOW hoặc ngược lại)
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_CHID(ChannelId);
    uint16 Pin = DIO_GET_PIN_MASK(ChannelId);  /* Sử dụng DIO_GET_PIN_MASK để lấy bit mask pin */

    if(Port == NULL_PTR) return STD_LOW;

    uint16 currentValue = GPIO_ReadInputDataBit(Port, Pin);
    if(currentValue == Bit_SET){
        GPIO_ResetBits(Port, Pin);
        return STD_LOW;        
    }
    else{
        GPIO_SetBits(Port, Pin);
        return STD_HIGH;        
    }
}

/**
 * @brief: Hàm ghi trạng thái cho 1 số kênh đầu ra được chọn trong 1 cổng DIO
 * @param[in] PortId: ID của cổng DIO cần ghi
 * @param[in] Level: Giá trị bitmask cần ghi cho các kênh được chọn
 * @param[in] Mask: Bitmask chọn các kênh để ghi
 * @details: Mask và ghi giá trị vào cổng GPIO
 */
void Dio_MaskedWritePort(Dio_PortType PortId, Dio_PortLevelType Level, Dio_PortLevelType Mask){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(PortId);

    if(Port == NULL_PTR) return;

    uint16 portValue = GPIO_ReadOutputData(Port);
    portValue = (portValue & ~Mask) | (Level & Mask);
    GPIO_Write(Port, portValue);
}
