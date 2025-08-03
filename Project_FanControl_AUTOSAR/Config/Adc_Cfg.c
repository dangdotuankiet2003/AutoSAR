#define ADC_ENABLED  // Đảm bảo bật #ifdef khi compile riêng

#include "Adc_Cfg.h"
#include "Adc.h"
#include "Dio.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

/* Biến toàn cục lưu trạng thái khởi tạo */
extern uint8 adcInit;  

/* Mảng ánh xạ kênh ADC */
const uint8 Adc_Channel_Mapping[10] = {
    ADC_Channel_0,  /* PA0 */
    ADC_Channel_1,  /* PA1 */
    ADC_Channel_2,  /* PA2 */
    ADC_Channel_3,  /* PA3 */
    ADC_Channel_4,  /* PA4 */
    ADC_Channel_5,  /* PA5 */
    ADC_Channel_6,  /* PA6 */
    ADC_Channel_7,  /* PA7 */
    ADC_Channel_8,  /* PB0 */
    ADC_Channel_9   /* PB1 */
};

#ifdef ADC_ENABLED  

/* Static con trỏ để truy cập cấu hình từ Adc.c */
extern Adc_ConfigType *adcConfigs;
extern Adc_GroupDefType *adcGroupConfigs;

/* Biến toàn cục để lưu cấu hình động và số nhóm */
static Adc_ConfigType AdcDynamicConfig[10];  // Tối đa 10 nhóm, đủ cho 10 kênh trong Adc_Channel_Mapping
static uint8 AdcDynamicNumGroups = 0;

/**
 * @brief: Tạo cấu hình ADC động
 * @param[in] adcInstance: Instance ADC (ADC_INSTANCE_1 hoặc ADC_INSTANCE_2)
 * @param[in] channelIds: Mảng các kênh ADC (DIO_CHANNEL_A0, A1, v.v.)
 * @param[in] numGroups: Số nhóm ADC (số kênh cần cấu hình)
 * @return: Con trỏ tới cấu hình ADC
 * @details: Sinh cấu hình ADC dựa trên thông tin từ application
 */
const Adc_ConfigType* Adc_CreateConfig(Adc_InstanceType adcInstance, const Adc_ChannelType *channelIds, uint8 numGroups)
{
    if (numGroups == 0 || numGroups > 10 || channelIds == NULL_PTR) {
        AdcDynamicNumGroups = 0;
        return NULL_PTR;
    }

    AdcDynamicNumGroups = numGroups;

    for (uint8 i = 0; i < numGroups; i++) {
        AdcDynamicConfig[i].ConversionMode = ADC_CONV_MODE_ONESHOT;
        AdcDynamicConfig[i].TriggerSource = ADC_TRIGGER_SOFTWARE;
        AdcDynamicConfig[i].NotificationEnabled = ADC_NOTIFICATION_DISABLE;
        AdcDynamicConfig[i].NumChannels = 1;
        AdcDynamicConfig[i].AdcInstance = adcInstance;
        AdcDynamicConfig[i].Channels[0].ChannelId = channelIds[i];
        AdcDynamicConfig[i].Channels[0].SampleTime = ADC_SampleTime_55Cycles5;
        AdcDynamicConfig[i].Channels[0].Rank = 1;
        AdcDynamicConfig[i].Initialback = NULL_PTR;
    }

    return AdcDynamicConfig;
}

/**
 * @brief: Lấy số lượng nhóm ADC
 * @return: Số nhóm ADC hiện tại
 */
uint8 Adc_GetNumGroups(void)
{
    return AdcDynamicNumGroups;
}

/**
 * @brief: Bật DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Cấu hình và bật DMA cho nhóm kênh sử dụng ADC1 trong chế độ streaming ([SWS_Adc_00125], trang 68)
 */
Std_ReturnType Adc_EnableDma(Adc_GroupType Group)
{
    if (!adcInit || adcGroupConfigs == NULL_PTR || Group >= AdcDynamicNumGroups) return E_NOT_OK;

    Adc_GroupDefType *grp = &adcGroupConfigs[Group];
    if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Adc_StreamEnableType == 0) return E_NOT_OK;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)grp->Result;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = grp->Adc_StreamBufferSize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = (grp->Adc_StreamBufferMode == ADC_STREAM_BUFFER_CIRCULAR) ? DMA_Mode_Circular : DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    ADC_DMACmd(ADC1, ENABLE);

    return E_OK;
}

/**
 * @brief: Tắt DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Tắt DMA và chế độ DMA của ADC cho nhóm kênh ([SWS_Adc_00125], trang 68)
 */
Std_ReturnType Adc_DisableDma(Adc_GroupType Group)
{
    if (!adcInit || adcGroupConfigs == NULL_PTR || Group >= AdcDynamicNumGroups) return E_NOT_OK;

    Adc_GroupDefType *grp = &adcGroupConfigs[Group];
    if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Adc_StreamEnableType == 0) return E_NOT_OK;

    ADC_DMACmd(ADC1, DISABLE);
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, DISABLE);
    NVIC_DisableIRQ(DMA1_Channel1_IRQn);

    return E_OK;
}

/**
 * @brief: Xử lý ngắt ADC
 * @details: Xử lý ngắt chung cho ADC1 và ADC2, xóa cờ ngắt và gọi callback ([SWS_Adc_00221], trang 76)
 */
void ADC1_2_IRQHandler(void)
{
    if (!adcInit || adcConfigs == NULL_PTR || adcGroupConfigs == NULL_PTR) return;

    for (uint8 i = 0; i < AdcDynamicNumGroups; i++) {
        Adc_GroupDefType *grp = &adcGroupConfigs[i];
        Adc_ConfigType *cfg = &adcConfigs[grp->AdcInstance];

        if (cfg->NotificationEnabled == ADC_NOTIFICATION_ENABLE && grp->Status == ADC_BUSY) {
            if (grp->AdcInstance == ADC_INSTANCE_1 && ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
                ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
                if (cfg->Initialback != NULL_PTR) cfg->Initialback();
                grp->Status = ADC_COMPLETED;
            } else if (grp->AdcInstance == ADC_INSTANCE_2 && ADC_GetITStatus(ADC2, ADC_IT_EOC)) {
                ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
                if (cfg->Initialback != NULL_PTR) cfg->Initialback();
                grp->Status = ADC_COMPLETED;
            }
        }
    }
}

/**
 * @brief: Xử lý ngắt DMA cho ADC1
 * @details: Xử lý ngắt DMA Transfer Complete cho ADC1, cập nhật trạng thái và gọi callback ([SWS_Adc_00221], trang 76)
 */
void DMA1_Channel1_IRQHandler(void)
{
    if (!adcInit || adcConfigs == NULL_PTR || adcGroupConfigs == NULL_PTR) return;

    for (uint8 i = 0; i < AdcDynamicNumGroups; i++) {
        Adc_GroupDefType *grp = &adcGroupConfigs[i];
        Adc_ConfigType *cfg = &adcConfigs[grp->AdcInstance];

        if (cfg->NotificationEnabled == ADC_NOTIFICATION_ENABLE && grp->Status == ADC_BUSY && grp->AdcInstance == ADC_INSTANCE_1) {
            if (DMA_GetITStatus(DMA1_IT_TC1)) {
                DMA_ClearITPendingBit(DMA1_IT_TC1);
                if (cfg->Initialback != NULL_PTR) cfg->Initialback();
                grp->Status = ADC_STREAM_COMPLETED;
            }
        }
    }
}

/**
 * @brief: Thiết lập bộ đệm DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[in] buf: Con trỏ tới bộ đệm DMA
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Gán bộ đệm DMA cho nhóm kênh sử dụng ADC1 trong chế độ streaming ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_SetupResultBuffer_Dma(Adc_GroupType Group, Adc_ValueGroupType* buf)
{
    if (!adcInit || adcGroupConfigs == NULL_PTR || Group >= AdcDynamicNumGroups || buf == NULL_PTR) return E_NOT_OK;

    Adc_GroupDefType *grp = &adcGroupConfigs[Group];
    if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Adc_StreamEnableType == 0) return E_NOT_OK;

    grp->Result = buf;
    return E_OK;
}

#endif  