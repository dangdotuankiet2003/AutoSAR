#ifndef ADC_CFG_H
#define ADC_CFG_H

#include "Adc.h"  

#ifdef ADC_ENABLED  

/* Mảng ánh xạ kênh ADC */
extern const uint8 Adc_Channel_Mapping[10];

/**
 * @brief: Tạo cấu hình ADC động
 * @param[in] adcInstance: Instance ADC (ADC_INSTANCE_1 hoặc ADC_INSTANCE_2)
 * @param[in] channelIds: Mảng các kênh ADC (DIO_CHANNEL_A0, A1, v.v.)
 * @param[in] numGroups: Số nhóm ADC (số kênh cần cấu hình)
 * @return: Con trỏ tới cấu hình ADC
 * @details: Sinh cấu hình ADC dựa trên thông tin từ application
 */
const Adc_ConfigType* Adc_CreateConfig(Adc_InstanceType adcInstance, const Adc_ChannelType *channelIds, uint8 numGroups);

/**
 * @brief: Lấy số lượng nhóm ADC
 * @return: Số nhóm ADC hiện tại
 */
uint8 Adc_GetNumGroups(void);

/**
 * @brief: Bật DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Cấu hình và bật DMA cho nhóm kênh sử dụng ADC1 trong chế độ streaming ([SWS_Adc_00125], trang 68)
 */
Std_ReturnType Adc_EnableDma(Adc_GroupType Group);

/**
 * @brief: Tắt DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Tắt DMA và chế độ DMA của ADC cho nhóm kênh ([SWS_Adc_00125], trang 68)
 */
Std_ReturnType Adc_DisableDma(Adc_GroupType Group);

/**
 * @brief: Xử lý ngắt ADC
 * @details: Xử lý ngắt chung cho ADC1 và ADC2, xóa cờ ngắt và gọi callback ([SWS_Adc_00221], trang 76)
 */
void ADC1_2_IRQHandler(void);

/**
 * @brief: Xử lý ngắt DMA cho ADC1
 * @details: Xử lý ngắt DMA Transfer Complete cho ADC1, cập nhật trạng thái và gọi callback ([SWS_Adc_00221], trang 76)
 */
void DMA1_Channel1_IRQHandler(void);

/**
 * @brief: Thiết lập bộ đệm DMA cho nhóm kênh ADC
 * @param[in] Group: ID của nhóm kênh
 * @param[in] buf: Con trỏ tới bộ đệm DMA
 * @return: Std_ReturnType - E_OK nếu thành công, E_NOT_OK nếu thất bại
 * @details: Gán bộ đệm DMA cho nhóm kênh sử dụng ADC1 trong chế độ streaming ([SWS_Adc_00318], trang 68)
 */
Std_ReturnType Adc_SetupResultBuffer_Dma(Adc_GroupType Group, Adc_ValueGroupType* buf);

#endif  // Kết thúc #ifdef ADC_ENABLED

#endif /* ADC_CFG_H */
