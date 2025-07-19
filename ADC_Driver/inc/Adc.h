#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

#define ADC_NUM_GROUPS 3
#define ADC_MAX_GROUPS 16
#define ADC_INSTANCE_1 0
#define ADC_INSTANCE_2 1
#define ADC_NOTIFICATION_ENABLE 1
#define ADC_NOTIFICATION_DISABLE 0

typedef uint8 Adc_ChannelType;
typedef uint8 Adc_GroupType;
typedef uint16 Adc_ValueGroupType;
typedef uint8 Adc_InstanceType;

typedef enum {
    ADC_IDLE,
    ADC_BUSY,
    ADC_COMPLETED,
    ADC_STREAM_COMPLETED
} Adc_StatusType;

typedef enum {
    ADC_TRIGGER_SOFTWARE,
    ADC_TRIGG_SRC_HW
} Adc_TriggerSourceType;

typedef enum {
    ADC_CONV_MODE_ONESHOT,
    ADC_CONV_MODE_CONTINUOUS
} Adc_GroupConvModeType;

typedef uint8 Adc_GroupPriorityType;
typedef uint8 Adc_StreamNumSampleType;

typedef enum {
    ADC_STREAM_BUFFER_LINEAR,
    ADC_STREAM_BUFFER_CIRCULAR
} Adc_StreamBufferModeType;

typedef uint8 Adc_NotificationType;
typedef void (*Adc_NotificationCbType)(void);

typedef struct {
    Adc_ChannelType ChannelId;
    uint32_t SampleTime;
    uint8_t Rank;
} Adc_ChannelConfigType;

typedef struct {
    Adc_GroupConvModeType ConversionMode;
    Adc_TriggerSourceType TriggerSource;
    Adc_NotificationType NotificationEnabled;
    uint8 NumChannels;
    Adc_InstanceType AdcInstance;
    Adc_ChannelConfigType Channels[16];
    Adc_NotificationCbType Initialback;
} Adc_ConfigType;

typedef struct {
    Adc_GroupType id;
    Adc_InstanceType AdcInstance;
    Adc_ChannelType Channels[16];
    Adc_GroupPriorityType Priority;
    uint8 NumChannels;
    Adc_StatusType Status;
    Adc_ValueGroupType* Result;
    uint8 Adc_StreamEnableType;
    uint8 Adc_StreamBufferSize;
    Adc_StreamBufferModeType Adc_StreamBufferMode;
} Adc_GroupDefType;

// Khai báo extern cho các biến toàn cục
extern Adc_ConfigType Adc_Configs[ADC_NUM_GROUPS];
extern Adc_GroupDefType Adc_GroupConfigs[ADC_MAX_GROUPS];

void Adc_Init(const Adc_ConfigType* ConfigPtr);
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, const Adc_ValueGroupType* DataBufferPtr);
void Adc_DeInit(void);
void Adc_StartGroupConversion(Adc_GroupType Group);
void Adc_StopGroupConversion(Adc_GroupType Group);
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);
void Adc_EnableHardwareTrigger(Adc_GroupType Group);
void Adc_DisableHardwareTrigger(Adc_GroupType Group);
void Adc_EnableGroupNotification(Adc_GroupType Group);
void Adc_DisableGroupNotification(Adc_GroupType Group);
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType** PtrToSamplePtr);
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif // ADC_H