#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"

/* @brief: Mảng ánh xạ kênh ADC
 * @details: Ánh xạ kênh ADC0-ADC9 tới kênh phần cứng STM32 (ADC_Channel_0 đến ADC_Channel_9)
 */
const uint8 Adc_Channel_Mapping[10] = {
    ADC_Channel_0, ADC_Channel_1, ADC_Channel_2, ADC_Channel_3, ADC_Channel_4,
    ADC_Channel_5, ADC_Channel_6, ADC_Channel_7, ADC_Channel_8, ADC_Channel_9
};

/* @brief: Bật DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Cấu hình và bật DMA cho nhóm kênh sử dụng ADC1 trong chế độ streaming ([SWS_Adc_00125], trang 68)
 */
Std_ReturnType Adc_EnableDma(Adc_GroupType Group) {
    // Kiểm tra nhóm và trạng thái khởi tạo
    if (!adcInit || Group >= ADC_NUM_GROUPS) return E_NOT_OK;
    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    // Kiểm tra nhóm hợp lệ và DMA được bật
    if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Status != ADC_IDLE || grp->Adc_StreamEnableType != 1 || grp->Result == NULL_PTR) return E_NOT_OK;

    // Bật clock cho DMA
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    // Vô hiệu hóa DMA trước khi cấu hình
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA_DeInit(DMA1_Channel1);

    // Cấu hình kênh DMA
    DMA_InitTypeDef DMA_InitStruct;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR; // Thanh ghi dữ liệu ADC1
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)grp->Result;   // Bộ đệm kết quả
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;              // Chuyển từ peripheral sang bộ nhớ
    DMA_InitStruct.DMA_BufferSize = grp->NumChannels;            // Kích thước bộ đệm
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // Không tăng địa chỉ peripheral
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;         // Tăng địa chỉ bộ nhớ
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // Kích thước dữ liệu 16-bit
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;                 // Chế độ vòng (circular)
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;             // Ưu tiên cao
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;                   // Không phải memory-to-memory
    DMA_Init(DMA1_Channel1, &DMA_InitStruct);

    // Bật ngắt DMA Transfer Complete
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    // Bật kênh DMA
    DMA_Cmd(DMA1_Channel1, ENABLE);
    // Bật chế độ DMA cho ADC1
    ADC_DMACmd(ADC1, ENABLE);
    // Cấu hình NVIC cho ngắt DMA
    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    return E_OK;
}

/* @brief: Tắt DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Tắt DMA và chế độ DMA của ADC cho nhóm kênh ([SWS_Adc_00125], trang 68)
 */
Std_ReturnType Adc_DisableDma(Adc_GroupType Group) {
    // Kiểm tra nhóm và trạng thái khởi tạo
    if (!adcInit || Group >= ADC_NUM_GROUPS) return E_NOT_OK;
    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    // Kiểm tra nhóm hợp lệ và DMA được bật
    if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Status != ADC_IDLE || grp->Adc_StreamEnableType != 1) return E_NOT_OK;

    // Tắt chế độ DMA của ADC
    ADC_DMACmd(ADC1, DISABLE);
    // Vô hiệu hóa kênh DMA
    DMA_Cmd(DMA1_Channel1, DISABLE);
    // Tắt ngắt DMA Transfer Complete
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, DISABLE);

    return E_OK;
}

/* @brief: Xử lý ngắt ADC
 * @details: Xử lý ngắt chung cho ADC1 và ADC2, xóa cờ ngắt và gọi callback ([SWS_Adc_00221], trang 76)
 */
void ADC1_2_IRQHandler(void) {
    // Lặp qua tất cả nhóm kênh
    for (uint8 group = 0; group < ADC_MAX_GROUPS; group++) {
        Adc_GroupDefType *grp = &Adc_GroupConfigs[group];
        Adc_ConfigType *cfg = &Adc_Configs[grp->AdcInstance];

        // Kiểm tra trạng thái thông báo và ngắt EOC
        if (cfg->NotificationEnabled == ADC_NOTIFICATION_ENABLE) {
            if (grp->AdcInstance == ADC_INSTANCE_1 && ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
                // Xóa cờ ngắt EOC cho ADC1
                ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
            }
            // Gọi callback nếu có
            if (cfg->Initialback)
                cfg->Initialback();
        }
        else if (grp->AdcInstance == ADC_INSTANCE_2 && ADC_GetITStatus(ADC2, ADC_IT_EOC)) {
            // Xóa cờ ngắt EOC cho ADC2
            ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
        }
        // Gọi callback nếu có
        if (cfg->Initialback)
            cfg->Initialback();
    }
}

/* @brief: Xử lý ngắt DMA cho ADC1
 * @details: Xử lý ngắt DMA Transfer Complete cho ADC1, cập nhật trạng thái và gọi callback ([SWS_Adc_00221], trang 76)
 */
void DMA1_Channel1_IRQHandler(void) {
    // Lặp qua tất cả nhóm kênh
    for (uint8 Group = 0; Group < ADC_NUM_GROUPS; Group++) {
        Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
        Adc_ConfigType* cfg = &Adc_Configs[grp->AdcInstance];
        // Kiểm tra nhóm hợp lệ
        if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Channels == NULL_PTR || grp->NumChannels == 0 || grp->Result == NULL_PTR) continue;

        // Kiểm tra và xóa cờ ngắt DMA Transfer Complete
        if (DMA_GetITStatus(DMA1_FLAG_TC1)) {
            DMA_ClearITPendingBit(DMA1_FLAG_TC1);
            // Cập nhật trạng thái nhóm thành STREAM_COMPLETED
            grp->Status = ADC_STREAM_COMPLETED;
            // Gọi callback nếu có
            if (cfg->Initialback != NULL_PTR) {
                cfg->Initialback();
            }
        }
    }
}

/* @brief: Thiết lập bộ đệm DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[in] buf: Con trỏ tới bộ đệm DMA
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Gán bộ đệm DMA cho nhóm kênh sử dụng ADC1 trong chế độ streaming ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_SetupResultBuffer_Dma(Adc_GroupType Group, Adc_ValueGroupType* buf) {
    // Kiểm tra nhóm, trạng thái khởi tạo và con trỏ
    if (!adcInit || Group >= ADC_NUM_GROUPS || buf == NULL_PTR) return E_NOT_OK;
    Adc_GroupDefType* grp = &Adc_GroupConfigs[Group];
    // Kiểm tra nhóm hợp lệ và DMA được bật
    if (grp->AdcInstance != ADC_INSTANCE_1 || grp->Status != ADC_IDLE || grp->Adc_StreamEnableType != 1) return E_NOT_OK;

    // Gán bộ đệm DMA
    grp->Result = buf;
    return E_OK;
}