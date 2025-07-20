#include "Dio.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_CHID(ChannelId);
    uint16 Pin = DIO_GET_PIN(ChannelId);

    if(Port == NULL_PTR){
        return STD_LOW;
    }
    
    return(GPIO_ReadInputDataBit(Port, Pin) == Bit_SET) ? STD_HIGH : STD_LOW;
}

void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level){
   GPIO_TypeDef *Port = DIO_GET_PORT_FR_CHID(ChannelId);
   uint16 Pin = DIO_GET_PIN(ChannelId);
  
    if(Port == NULL_PTR){
        return;
    }

    if(Level == STD_HIGH) GPIO_SetBits(Port, Pin);
    else GPIO_ResetBits(Port, Pin);
}

Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(PortId);
    if(Port == NULL_PTR) return null;
    return(Dio_PortLevelType)(GPIO_ReadInputData(Port));
}

void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(PortId);
    if(Port == NULL_PTR) return;
    GPIO_Write(Port, Level);
}

Dio_PortLevelType Dio_ReadChannelGroup (const Dio_ChannelGroupType* ChannelGroupIdPtr){
   GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(ChannelGroupIdPtr->port);
   if(Port == NULL_PTR) return null;
   uint16 portValue = GPIO_ReadInputData(Port);

   return (Dio_PortLevelType)((portValue & ChannelGroupIdPtr->mask) >> ChannelGroupIdPtr->offset);
}

void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(ChannelGroupIdPtr->port);
    if(Port == NULL_PTR) return;
    
    uint16_t portValue = GPIO_ReadInputData(Port);
    // Xóa các bit trong nhóm, sau đó ghi giá trị mới vào trong nhóm kênh
    portValue &= ~(ChannelGroupIdPtr->mask); //Reset tương ứng với các bit mask
    portValue |= ((Level << ChannelGroupIdPtr->offset) & (ChannelGroupIdPtr->mask));
    
    GPIO_Write(Port, portValue);
}  

void Dio_GetVersionInfo (Std_VersionInfoType* VersionInfo){

    VersionInfo->moduleID = 0x0001;
    VersionInfo->vendorID = 0x0002;
    VersionInfo->sw_major_version = 1;
    VersionInfo->sw_minor_version = 0;
    VersionInfo->sw_patch_version = 0;
}


Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_CHID(ChannelId);
    uint16 Pin = DIO_GET_PIN(ChannelId);

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

void Dio_MaskedWritePort (Dio_PortType PortId, Dio_PortLevelType Level, Dio_PortLevelType Mask){
    GPIO_TypeDef *Port = DIO_GET_PORT_FR_PORTID(PortId);

    if(Port == NULL_PTR) return;

    uint16 portValue = GPIO_ReadOutputData(Port);
    portValue = (portValue &~ Mask) | (Level & Mask);
    GPIO_Write(Port, portValue);
}