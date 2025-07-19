#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "misc.h"

// Định nghĩa các biến toàn cục
Adc_ValueGroupType* Adc_DmaBuffer[ADC_NUM_GROUPS];
uint8 adcInit = 0;

// Khai báo extern cho Adc_Configs và Adc_GroupConfigs
extern Adc_ConfigType Adc_Configs[ADC_NUM_GROUPS];
extern Adc_GroupDefType Adc_GroupConfigs[ADC_MAX_GROUPS];
extern const uint8 Adc_Channel_Mapping[10];

/* @brief: Khởi tạo ADC Driver
 * @param[in] ConfigPtr: Con trỏ tới cấu hình toàn cục ADC
 * @details: Khởi tạo ADC hardware units và ADC driver ([SWS_Adc_00092], trang 25)
 */
void Adc_Init(const Adc_ConfigType* ConfigPtr) {
    if (ConfigPtr == NULL_PTR || adcInit) return;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    // Cấu hình clock ADC thành PCLK2/6
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    for (uint8 i = 0; i < ADC_NUM_GROUPS; i++) {
        const Adc_ConfigType* cfg = &ConfigPtr[i];
        ADC_TypeDef* ADCx = (cfg->AdcInstance == ADC_INSTANCE_1) ? ADC1 : ADC2;

        Adc_GroupConfigs[i].id = i;
        Adc_GroupConfigs[i].AdcInstance = cfg->AdcInstance;
        Adc_GroupConfigs[i].NumChannels = cfg->NumChannels;
        Adc_GroupConfigs[i].Status = ADC_IDLE;
        Adc_GroupConfigs[i].Result = NULL_PTR;
        Adc_GroupConfigs[i].Adc_StreamEnableType = (cfg->ConversionMode == ADC_CONV_MODE_CONTINUOUS) ? 1 : 0;
        Adc_GroupConfigs[i].Adc_StreamBufferMode = ADC_STREAM_BUFFER_CIRCULAR;
        Adc_GroupConfigs[i].Priority = 0;

        // Lưu danh sách kênh vào cấu hình nhóm
        for (uint8 j = 0; j < cfg->NumChannels; j++) {
            Adc_GroupConfigs[i].Channels[j] = cfg->Channels[j].ChannelId;
        }

        // Cấu hình các kênh ADC
        for (uint8 j = 0; j < cfg->NumChannels; j++) {
            if (cfg->Channels[j].ChannelId < 10) {
                ADC_RegularChannelConfig(ADCx, Adc_Channel_Mapping[cfg->Channels[j].ChannelId], cfg->Channels[j].Rank, cfg->Channels[j].SampleTime);
            }
        }

        // Cấu hình ADC hardware
        ADC_InitTypeDef ADC_InitStructure;
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ContinuousConvMode = (cfg->ConversionMode == ADC_CONV_MODE_CONTINUOUS) ? ENABLE : DISABLE;
        ADC_InitStructure.ADC_ScanConvMode = (cfg->NumChannels > 1) ? ENABLE : DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConv = (cfg->TriggerSource == ADC_TRIGGER_SOFTWARE) ? ADC_ExternalTrigConv_None : ADC_ExternalTrigConv_T1_CC1;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel = cfg->NumChannels;

        // Áp dụng cấu hình ADC
        ADC_Init(ADCx, &ADC_InitStructure);
        ADC_Cmd(ADCx, ENABLE);
        // Hiệu chỉnh ADC
        ADC_ResetCalibration(ADCx);
        while(ADC_GetResetCalibrationStatus(ADCx));
        ADC_StartCalibration(ADCx);
        while(ADC_GetCalibrationStatus(ADCx));
    }
    // Set flag báo ADC đã được khởi tạo
    adcInit = 1;
}

/* @brief: Thiết lập bộ đệm kết quả cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[in] DataBufferPtr: Con trỏ tới bộ đệm lưu các mẫu ADC
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Gán bộ đệm để lưu kết quả chuyển đổi ADC ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, const Adc_ValueGroupType* DataBufferPtr) {
    if (Group >= ADC_MAX_GROUPS || DataBufferPtr == NULL_PTR) return E_NOT_OK;
    
    Adc_GroupConfigs[Group].Result = (Adc_ValueGroupType*)DataBufferPtr;
    return E_OK;
}

/* @brief: Hủy khởi tạo ADC Driver
 * @details: Tắt ADC hardware unit và đặt lại trạng thái ban đầu ([SWS_Adc_00358], trang 67)
 */
void Adc_DeInit(void) {
    if (!adcInit) return;

    for (uint8 Group = 0; Group < ADC_NUM_GROUPS; Group++) {
        if (Adc_GroupConfigs[Group].Status != ADC_IDLE) return;
    }
    // Tắt ngắt và module ADC1, ADC2
    ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    ADC_Cmd(ADC1, DISABLE);
    ADC_DeInit(ADC1);

    ADC_ITConfig(ADC2, ADC_IT_EOC, DISABLE);
    ADC_Cmd(ADC2, DISABLE);
    ADC_DeInit(ADC2);

    // Tắt clock ADC và DMA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, DISABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);

    // Đặt lại trạng thái khởi tạo và cấu hình nhóm
    adcInit = 0;
    for (uint8 Group = 0; Group < ADC_NUM_GROUPS; Group++) {
        Adc_GroupConfigs[Group].Result = NULL_PTR;
        Adc_GroupConfigs[Group].Status = ADC_IDLE;
        Adc_GroupConfigs[Group].Adc_StreamEnableType = 0;
        Adc_GroupConfigs[Group].NumChannels = 0;
    }
}

/* @brief: Bắt đầu chuyển đổi nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Kích hoạt chuyển đổi ADC cho nhóm kênh ([SWS_Adc_00061], trang 64)
 */
void Adc_StartGroupConversion(Adc_GroupType Group) {
    Adc_GroupDefType *grp = &Adc_GroupConfigs[Group];
    if (grp->Status != ADC_IDLE) return;

    // Đặt trạng thái nhóm thành BUSY
    grp->Status = ADC_BUSY;

    // Kích hoạt chuyển đổi bằng phần mềm
    if (grp->AdcInstance == ADC_INSTANCE_1) {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    } else {
        ADC_SoftwareStartConvCmd(ADC2, ENABLE);
    }
}

/* @brief: Dừng chuyển đổi nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Dừng chuyển đổi ADC và đặt nhóm về trạng thái IDLE ([SWS_Adc_00062], trang 65)
 */
void Adc_StopGroupConversion(Adc_GroupType Group) {
    if (!adcInit || Group >= ADC_NUM_GROUPS) return;

    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    // Kiểm tra nhóm hợp lệ và ở chế độ software trigger
    if (grp->Channels == NULL_PTR || grp->NumChannels == 0 || grp->Status != ADC_BUSY || Adc_Configs[grp->AdcInstance].TriggerSource != ADC_TRIGGER_SOFTWARE) return;
    
    // Dừng chuyển đổi bằng phần mềm
    if (grp->AdcInstance == ADC_INSTANCE_1) {
        ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    }
    else if(grp->AdcInstance == ADC_INSTANCE_2){
        ADC_SoftwareStartConvCmd(ADC2, DISABLE);
    }

    // Đặt trạng thái nhóm về IDLE
    grp->Status = ADC_IDLE;
}

/* @brief: Đọc kết quả chuyển đổi nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[out] DataBufferPtr: Con trỏ tới bộ đệm lưu kết quả ADC
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Đọc giá trị ADC từ bộ đệm sau khi chuyển đổi hoàn tất ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr) {
    Adc_GroupDefType *grp = &Adc_GroupConfigs[Group];
    Adc_ConfigType *cfg = &Adc_Configs[grp->AdcInstance];
    
    // Kiểm tra trạng thái nhóm hợp lệ
    if (grp->Status != ADC_BUSY && grp->Status != ADC_STREAM_COMPLETED)
        return E_NOT_OK;

    // Đọc giá trị từ thanh ghi dữ liệu ADC
    if (grp->AdcInstance == ADC_INSTANCE_1)
        *DataBufferPtr = ADC_GetConversionValue(ADC1);
    else
        *DataBufferPtr = ADC_GetConversionValue(ADC2);

    // Đặt lại trạng thái để cho phép chuyển đổi mới
    grp->Status = ADC_IDLE; 

    // Lưu giá trị vào bộ đệm nếu có
    if (grp->Result != NULL_PTR)
        grp->Result[0] = *DataBufferPtr;

    return E_OK;
}

/* @brief: Kích hoạt trigger phần cứng cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Kích hoạt trigger phần cứng để bắt đầu chuyển đổi ([SWS_Adc_00147], trang 69)
 */
void Adc_EnableHardwareTrigger(Adc_GroupType Group) {
    // Kiểm tra nhóm và trạng thái khởi tạo
    if (!adcInit || Group >= ADC_NUM_GROUPS) return;

    // Kiển tra nhóm có hợp lệ và dùng hardware trigger hay không
    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    if (grp->Channels == NULL_PTR || grp->NumChannels == 0 || grp->Status != ADC_IDLE || Adc_Configs[grp->AdcInstance].TriggerSource != ADC_TRIGG_SRC_HW) return;

    grp->Status = ADC_BUSY;

    // Cấu hình trigger phần cứng
    ADC_TypeDef *ADCx = (grp->AdcInstance == ADC_INSTANCE_1) ? ADC1 : ADC2;
    ADC_Cmd(ADCx, ENABLE);
    ADCx->CR2 &= ~(0x7 << 17); // Xóa lựa chọn trigger
    ADCx->CR2 |= (0x1 << 17);  // Chọn nguồn trigger
    ADCx->CR2 |= (1UL << 20);  // Bật trigger ngoài
}

/* @brief: Vô hiệu hóa trigger phần cứng cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Vô hiệu hóa trigger phần cứng, dừng chuyển đổi ([SWS_Adc_00148], trang 70)
 */
void Adc_DisableHardwareTrigger(Adc_GroupType Group) {
    // Kiểm tra điều kiện nhóm
    if(!adcInit || Group >= ADC_NUM_GROUPS) return;

    // Kiểm tra nhóm hợp lệ và ở chế độ hardware trigger
    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    if (grp->Channels == NULL_PTR || grp->NumChannels == 0 || grp->Status != ADC_BUSY || Adc_Configs[grp->AdcInstance].TriggerSource != ADC_TRIGG_SRC_HW) return;

    // Tắt external trigger
    ADC_TypeDef* ADCx = (grp->AdcInstance == ADC_INSTANCE_1) ? ADC1 : ADC2;
    ADCx->CR2 &= ~(1UL << 20);
    // Đặt trạng thám nhóm về ADC_IDLE
    grp->Status = ADC_IDLE;
}

/* @brief: Bật thông báo cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Bật callback khi nhóm hoàn tất chuyển đổi ([SWS_Adc_00149], trang 71)
 */
void Adc_EnableGroupNotification(Adc_GroupType Group) {
    Adc_ConfigType *cfg = &Adc_Configs[Adc_GroupConfigs[Group].AdcInstance];
    // Cho phép bật thông báo ngắt
    cfg->NotificationEnabled = ADC_NOTIFICATION_ENABLE;

    // Cấu hình ngắt cho ADCx
    if (Adc_GroupConfigs[Group].AdcInstance == ADC_INSTANCE_1) {
        ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
        NVIC_SetPriority(ADC1_2_IRQn, 0);
        NVIC_EnableIRQ(ADC1_2_IRQn);
    } else {
        ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);
        NVIC_SetPriority(ADC1_2_IRQn, 0);
        NVIC_EnableIRQ(ADC1_2_IRQn);
    }
}

/* @brief: Tắt thông báo cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @details: Tắt callback cho nhóm kênh ([SWS_Adc_00150], trang 72)
 */
void Adc_DisableGroupNotification(Adc_GroupType Group) {
    // Kiểm tra điều kiện nhóm
    if (!adcInit || Group >= ADC_NUM_GROUPS) return;

    // Tắt thông báo ngắt
    Adc_ConfigType* cfg = &Adc_Configs[Adc_GroupConfigs[Group].AdcInstance];
    cfg->NotificationEnabled = ADC_NOTIFICATION_DISABLE;

    // Tắt ngắt ADCx
    if (Adc_GroupConfigs[Group].AdcInstance == ADC_INSTANCE_1) {
        ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
    }
    else ADC_ITConfig(ADC2, ADC_IT_EOC, DISABLE);
}

/* @brief: Lấy con trỏ tới mẫu cuối cùng trong bộ đệm streaming
 * @param[in] Group: ID của nhóm kênh
 * @param[out] PtrToSamplePtr: Con trỏ tới con trỏ bộ đệm chứa mẫu cuối cùng
 * @return: Adc_StreamNumSampleType - Số mẫu hợp lệ, hoặc 0 nếu lỗi
 * @details: Trả về con trỏ tới mẫu cuối cùng và số mẫu trong chế độ streaming ([SWS_Adc_00324], trang 74)
 */
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType** PtrToSamplePtr) {
    // Kiểm tra nhóm, trạng thái khởi tạo và con trỏ
    if (!adcInit || Group >= ADC_NUM_GROUPS || PtrToSamplePtr == NULL_PTR) return 0;

    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    Adc_ConfigType* cfg = &Adc_Configs[grp->AdcInstance];
    // Kiểm tra nhóm hợp lệ và ở chế độ streaming
    if (grp->Channels == NULL_PTR || grp->NumChannels == 0 || grp->Result == NULL_PTR || cfg->ConversionMode != ADC_CONV_MODE_CONTINUOUS || grp->Status != ADC_STREAM_COMPLETED) return 0;

    // Trả về con trỏ tới bộ đệm kết quả
    *PtrToSamplePtr = grp->Result;
    return grp->NumChannels;
}

/* @brief: Lấy trạng thái của nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Adc_StatusType - Trạng thái của nhóm
 * @details: Trả về trạng thái hiện tại của nhóm kênh ([SWS_Adc_00059], trang 73)
 */
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group) {
    // Kiểm tra nhóm và trạng thái khởi tạo
    if (!adcInit || Group >= ADC_NUM_GROUPS) return ADC_IDLE;
    return Adc_GroupConfigs[Group].Status;
}

/* @brief: Lấy thông tin phiên bản của ADC Driver
 * @param[out] VersionInfo: Con trỏ tới thông tin phiên bản
 * @details: Cung cấp thông tin phiên bản của module ([SWS_Adc_00365], trang 82)
 */
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo == NULL_PTR) return;
    VersionInfo->moduleID = 0x01;
    VersionInfo->vendorID = 0x01;
    VersionInfo->sw_major_version = 0x01;
    VersionInfo->sw_minor_version = 0x00;
    VersionInfo->sw_patch_version = 0x00;
}