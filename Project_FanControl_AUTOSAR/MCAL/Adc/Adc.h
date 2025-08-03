#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

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

/**
 * @brief: Khởi tạo ADC Driver
 * @param[in] ConfigPtr: Con trỏ tới cấu hình toàn cục ADC
 * @param[in] GroupPtr: Con trỏ tới cấu hình nhóm ADC
 * @details: Khởi tạo ADC hardware units và ADC driver ([SWS_Adc_00092], trang 25)
 */
void Adc_Init(const Adc_ConfigType* ConfigPtr, const Adc_GroupDefType* GroupPtr);

/**
 * @brief: Thiết lập bộ đệm kết quả cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[in] DataBufferPtr: Con trỏ tới bộ đệm lưu các mẫu ADC
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Gán bộ đệm để lưu kết quả chuyển đổi ADC ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, const Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief: Hủy khởi tạo ADC Driver
 * @details: Tắt ADC hardware unit và đặt lại trạng thái ban đầu ([SWS_Adc_00358], trang 67)
 */
void Adc_DeInit(void);

/**
 * @brief: Bắt đầu chuyển đổi nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Kích hoạt chuyển đổi ADC cho nhóm kênh ([SWS_Adc_00061], trang 64)
 */
void Adc_StartGroupConversion(Adc_GroupType Group);

/**
 * @brief: Dừng chuyển đổi nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Dừng chuyển đổi ADC và đặt nhóm về trạng thái IDLE ([SWS_Adc_00062], trang 65)
 */
void Adc_StopGroupConversion(Adc_GroupType Group);

/**
 * @brief: Đọc kết quả chuyển đổi nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[out] DataBufferPtr: Con trỏ tới bộ đệm lưu kết quả ADC
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Đọc giá trị ADC từ bộ đệm sau khi chuyển đổi hoàn tất ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

/**
 * @brief: Kích hoạt trigger phần cứng cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Kích hoạt trigger phần cứng để bắt đầu chuyển đổi ([SWS_Adc_00147], trang 69)
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief: Vô hiệu hóa trigger phần cứng cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Vô hiệu hóa trigger phần cứng, dừng chuyển đổi ([SWS_Adc_00148], trang 70)
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**
 * @brief: Bật thông báo cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Bật callback khi nhóm hoàn tất chuyển đổi ([SWS_Adc_00149], trang 71)
 */
void Adc_EnableGroupNotification(Adc_GroupType Group);

/**
 * @brief: Tắt thông báo cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Tắt callback cho nhóm kênh ([SWS_Adc_00150], trang 72)
 */
void Adc_DisableGroupNotification(Adc_GroupType Group);

/**
 * @brief: Lấy con trỏ tới mẫu cuối cùng trong bộ đệm streaming
 * @param[in] Group: ID của nhóm kênh
 * @param[out] PtrToSamplePtr: Con trỏ tới con trỏ bộ đệm chứa mẫu cuối cùng
 * @return: Adc_StreamNumSampleType - Số mẫu hợp lệ, hoặc 0 nếu lỗi
 * @details: Trả về con trỏ tới mẫu cuối cùng và số mẫu trong chế độ streaming ([SWS_Adc_00324], trang 74)
 */
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType** PtrToSamplePtr);

/**
 * @brief: Lấy trạng thái của nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Adc_StatusType - Trạng thái của nhóm
 * @details: Trả về trạng thái hiện tại của nhóm kênh ([SWS_Adc_00059], trang 73)
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**
 * @brief: Lấy thông tin phiên bản của ADC Driver
 * @param[out] VersionInfo: Con trỏ tới thông tin phiên bản
 * @details: Cung cấp thông tin phiên bản của module ([SWS_Adc_00365], trang 82)
 */
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* ADC_H */