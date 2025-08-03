#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Std_Types.h"
#include "Pwm.h"
#include "Pwm_Cfg.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

/* Biến nội bộ để lưu cấu hình sau khi khởi tạo */
static const Pwm_ConfigType* pwmConfig = NULL_PTR;

/* Mảng static để lưu trạng thái notification cho từng kênh (tối đa 4 kênh) */
static uint8 notificationStates[4] = {0};

/* @brief: Hàm khởi tạo PWM driver
 * @param[in] ConfigPtr: Con trỏ đến cấu hình PWM
 * @details: Khởi tạo biến nội bộ và phần cứng PWM theo cấu hình
 */
void Pwm_Init(const Pwm_ConfigType* ConfigPtr){
    // Kiểm tra ConfigPtr hợp lệ, số kênh từ 1 đến 4
    if (ConfigPtr == NULL_PTR || ConfigPtr->numChannels == 0 || ConfigPtr->numChannels > 4) {
        return;
    }
    pwmConfig = ConfigPtr;

    /* Reset giá trị ban đầu mảng notificationStates lưu trạng thái thông báo của kênh PWM */
    memset(notificationStates, 0, sizeof(notificationStates));

    /* Duyệt qua từng kênh để khởi tạo cấu hình */
    for(uint8 i = 0; i < pwmConfig->numChannels; i++){
        const Pwm_ChannelConfigType *ch = &pwmConfig->channels[i];
        TIM_TypeDef *TIMx = ch->TIMx;

        // Bỏ qua nếu TIMx NULL
        if (TIMx == NULL_PTR) {
            continue;
        }

        /* Bật clock cho Timer */
        if(TIMx == TIM2)        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        else if(TIMx == TIM3)   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        else if(TIMx == TIM4)   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

        /* Chọn chế độ Output Compare dựa vào polarity */
        uint16 ocMode = (ch->polarity == PWM_HIGH) ? TIM_OCMode_PWM1 : TIM_OCMode_PWM2;

        /* Cấu hình Time Base */
        TIM_TimeBaseInitTypeDef timBase;
        timBase.TIM_Prescaler = ch->prescaler;        // Set prescaler (e.g., 71 cho 10kHz)
        timBase.TIM_Period = ch->defaultPeriod - 1;   /* ARR = period - 1 */
        timBase.TIM_ClockDivision = 0;                // Không chia tần số
        timBase.TIM_CounterMode = TIM_CounterMode_Up; // Đếm lên
        TIM_TimeBaseInit(TIMx, &timBase);
        TIM_ARRPreloadConfig(TIMx, ENABLE);           // Bật preload cho ARR

        /* Cấu hình Output Compare */
        TIM_OCInitTypeDef ocInitStructure;
        ocInitStructure.TIM_OCMode = ocMode; // Chọn PWM mode 1 hay PWM mode 2 dựa trên polarity
        ocInitStructure.TIM_OutputState = TIM_OutputState_Enable; // Cho phép kênh output compare trong PWM
        ocInitStructure.TIM_Pulse = (uint32_t)((uint32_t)ch->defaultDutyCycle * (timBase.TIM_Period + 1)) / 0x8000; // Tính giá trị CCR
        ocInitStructure.TIM_OCPolarity = (ch->polarity == PWM_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low; 

        /* Khởi tạo các kênh theo ID */
        if(ch->channel == 1) {
            TIM_OC1Init(TIMx, &ocInitStructure);
            TIMx->CCER |= TIM_CCER_CC1E;  // Enable channel 1 
        }
        else if(ch->channel == 2) {
            TIM_OC2Init(TIMx, &ocInitStructure);
            TIMx->CCER |= TIM_CCER_CC2E; // Enable channel 2 
        }
        else if(ch->channel == 3) {
            TIM_OC3Init(TIMx, &ocInitStructure);
            TIMx->CCER |= TIM_CCER_CC3E; // Enable channel 3 
        }
        else if(ch->channel == 4) {
            TIM_OC4Init(TIMx, &ocInitStructure);
            TIMx->CCER |= TIM_CCER_CC4E; // Enable channel 4 
        }

        /* Bật preload cho CCR - lưu giá trị ccr vào buffer trước */
        if(ch->channel == 1) TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
        else if(ch->channel == 2) TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
        else if(ch->channel == 3) TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
        else if(ch->channel == 4) TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);

        /* Bật Timer */
        TIM_Cmd(TIMx, ENABLE);

        /* Bật ngắt nếu enable */
        if(ch->notificationEnable){
            uint16 channelITFlag = TIM_IT_CC1 << (ch->channel - 1);
            TIM_ITConfig(TIMx, channelITFlag | TIM_IT_Update, ENABLE);
        }

        notificationStates[i] = ch->notificationEnable;
    }
}

/* @brief: Hàm hủy khởi tạo PWM driver
 * @details: Đặt PWM về trạng thái ban đầu, tắt ngắt và thông báo
 */
void Pwm_DeInit(void){
    // Kiểm tra pwmConfig hợp lệ
    if(pwmConfig == NULL_PTR) return;

    // Duyệt qua các kênh để tắt
    for(uint8 i = 0; i < pwmConfig->numChannels; i++){
        const Pwm_ChannelConfigType *ch = &pwmConfig->channels[i];
        TIM_TypeDef *TIMx = ch->TIMx;

        // Bỏ qua nếu TIMx null
        if (TIMx == NULL_PTR) continue;

        TIM_Cmd(TIMx, DISABLE);  // Tắt timer

        // Tắt ngắt nếu đang bật
        if(notificationStates[i]){
            Pwm_DisableNotification(i);
            notificationStates[i] = 0;
        }

        // Tắt clock timer
        if (TIMx == TIM2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
        else if (TIMx == TIM3) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
        else if (TIMx == TIM4) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, DISABLE);
    }

    // Reset notificationStates
    memset(notificationStates, 0, sizeof(notificationStates));
    pwmConfig = NULL_PTR;
}

/* @brief: Hàm đặt duty cycle cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] DutyCycle: Giá trị duty cycle (0x0000: 0%, 0x8000: 100%)
 * @details: Cập nhật duty cycle mà không thay đổi chu kỳ
 */
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle){
    // Kiểm tra pwmConfig và ChannelNumber hợp lệ
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) {
        return;
    }

    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    // Bỏ qua nếu TIMx null
    if (TIMx == NULL_PTR) {
        return;
    }

    /* Kiểm tra timer đã bật chưa */
    // TIM_CR1_CEN là bit Counter Enable (bit 0 trong TIMx->CR1), quyết định timer có đang chạy hay không
    if ((TIMx->CR1 & TIM_CR1_CEN) == 0) {
        return;
    }

    /* Xác định chu kì đếm của PWM - ARR: Auto Reload Register */
    uint32_t arr = TIMx->ARR;
    /* Xác định điểm mà xung PWM chuyển trạng thái từ HIGH -> LOW và ngược lại */
    uint32_t ccr = ((uint32_t)DutyCycle * (arr + 1)) / 0x8000;

    /* Thiết lập giá trị ccr vào thanh ghi Capture/Compare Register dựa vào id kênh */
    if (ch->channel == 1) TIM_SetCompare1(TIMx, ccr);
    else if(ch->channel == 2) TIM_SetCompare2(TIMx, ccr);
    else if(ch->channel == 3) TIM_SetCompare3(TIMx, ccr);
    else if(ch->channel == 4) TIM_SetCompare4(TIMx, ccr);
}

/* @brief: Hàm đặt chu kỳ và duty cycle cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] Period: Giá trị chu kỳ (ticks)
 * @param[in] DutyCycle: Giá trị duty cycle (0x0000: 0%, 0x8000: 100%)
 * @details: Chỉ áp dụng cho kênh lớp PWM_VARIABLE_PERIOD
 */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16 DutyCycle){
    // Kiểm tra pwmConfig và ChannelNumber hợp lệ
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) {
        return;
    }
    
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    // Bỏ qua nếu TIMx null
    if (TIMx == NULL_PTR) {
        return;
    }

    // Chỉ áp dụng cho kênh PWM_VARIABLE_PERIOD
    if(ch->channelClass != PWM_VARIABLE_PERIOD) return;

    // Set chu kỳ PWM (ARR)
    uint32_t arr = Period - 1;
    // Tính CCR từ DutyCycle
    uint32_t ccr = ((uint32_t)DutyCycle * (arr + 1)) / 0x8000;

    TIM_SetAutoreload(TIMx, arr);  // Cập nhật ARR

    // Set CCR cho kênh
    if (ch->channel == 1) {
        TIM_SetCompare1(TIMx, ccr);
    }
    else if(ch->channel == 2) TIM_SetCompare2(TIMx, ccr);
    else if(ch->channel == 3) TIM_SetCompare3(TIMx, ccr);
    else if(ch->channel == 4) TIM_SetCompare4(TIMx, ccr);
}

/* @brief: Hàm đặt đầu ra PWM về trạng thái idle
 * @param[in] ChannelNumber: Số kênh PWM
 * @details: Đặt đầu ra về trạng thái idle đã cấu hình (high/low)
 */
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber){
    // Kiểm tra pwmConfig và ChannelNumber hợp lệ
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;
    
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    // Bỏ qua nếu TIMx null
    if (TIMx == NULL_PTR) return;

    // Tắt output kênh PWM
    if (ch->channel == 1) TIM_CCxCmd(TIMx, TIM_Channel_1, TIM_CCx_Disable);
    else if (ch->channel == 2) TIM_CCxCmd(TIMx, TIM_Channel_2, TIM_CCx_Disable);
    else if (ch->channel == 3) TIM_CCxCmd(TIMx, TIM_Channel_3, TIM_CCx_Disable);
    else if (ch->channel == 4) TIM_CCxCmd(TIMx, TIM_Channel_4, TIM_CCx_Disable);

    GPIO_TypeDef *GPIOx = NULL_PTR;
    uint16 gpioPin = 0;

    // Map pin cho TIM2
    if(TIMx == TIM2){
        if(ch->channel == 1){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_0; /* TIM2_CH1: PA0 */
        }
        else if(ch->channel == 2){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_1; /* TIM2_CH2: PA1 */
        }
        else if(ch->channel == 3){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_2; /* TIM2_CH3: PA2 */
        }
        else if(ch->channel == 4){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_3; /* TIM2_CH4: PA3 */
        }
    }
    // Map pin cho TIM3
    else if(TIMx == TIM3){
        if(ch->channel == 1){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_6; /* TIM3_CH1: PA6 */
        }
        else if(ch->channel == 2){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_7; /* TIM3_CH2: PA7 */
        }
        else if(ch->channel == 3){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_0; /* TIM3_CH3: PB0 */
        }
        else if(ch->channel == 4){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_1; /* TIM3_CH4: PB1 */
        }
    }
    // Map pin cho TIM4
    else if(TIMx == TIM4){
        if(ch->channel == 1){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_6; /* TIM4_CH1: PB6 */
        }
        else if(ch->channel == 2){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_7; /* TIM4_CH2: PB7 */
        }
        else if(ch->channel == 3){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_8; /* TIM4_CH3: PB8 */
        }
        else if(ch->channel == 4){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_9; /* TIM4_CH4: PB9 */
        }
    }

    // Set pin về trạng thái idle (HIGH hoặc LOW)
    if (GPIOx != NULL_PTR) {
        if (ch->idleState == PWM_HIGH) GPIO_SetBits(GPIOx, gpioPin);
        else GPIO_ResetBits(GPIOx, gpioPin);
    }
}

/* @brief: Hàm lấy trạng thái đầu ra PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @return: Trạng thái đầu ra (PWM_HIGH hoặc PWM_LOW)
 * @details: Trả về mức logic hiện tại của đầu ra PWM
 */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber){
    // Kiểm tra pwmConfig và ChannelNumber hợp lệ
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return PWM_LOW;
    
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    // Bỏ qua nếu TIMx null
    if (TIMx == NULL_PTR) return PWM_LOW;

    GPIO_TypeDef *GPIOx = NULL_PTR;
    uint16 gpioPin = 0;

    // Map pin cho TIM2
    if(TIMx == TIM2){
        if(ch->channel == 1){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_0; /* TIM2_CH1: PA0 */
        }
        else if(ch->channel == 2){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_1; /* TIM2_CH2: PA1 */
        }
        else if(ch->channel == 3){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_2; /* TIM2_CH3: PA2 */
        }
        else if(ch->channel == 4){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_3; /* TIM2_CH4: PA3 */
        }
    }
    // Map pin cho TIM3
    else if(TIMx == TIM3){
        if(ch->channel == 1){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_6; /* TIM3_CH1: PA6 */
        }
        else if(ch->channel == 2){
            GPIOx = GPIOA;
            gpioPin = GPIO_Pin_7; /* TIM3_CH2: PA7 */
        }
        else if(ch->channel == 3){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_0; /* TIM3_CH3: PB0 */
        }
        else if(ch->channel == 4){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_1; /* TIM3_CH4: PB1 */
        }
    }
    // Map pin cho TIM4
    else if(TIMx == TIM4){
        if(ch->channel == 1){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_6; /* TIM4_CH1: PB6 */
        }
        else if(ch->channel == 2){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_7; /* TIM4_CH2: PB7 */
        }
        else if(ch->channel == 3){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_8; /* TIM4_CH3: PB8 */
        }
        else if(ch->channel == 4){
            GPIOx = GPIOB;
            gpioPin = GPIO_Pin_9; /* TIM4_CH4: PB9 */
        }
    }

    // Đọc trạng thái pin
    if (GPIOx != NULL_PTR){
        if(GPIO_ReadOutputDataBit(GPIOx, gpioPin) == Bit_SET) return PWM_HIGH;
        else return PWM_LOW;
    }

    return PWM_LOW;
}

/* @brief: Hàm tắt thông báo ngắt cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @details: Tắt thông báo cạnh cho kênh cụ thể
 */
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber){
    // Kiểm tra pwmConfig và ChannelNumber hợp lệ
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;
    
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    // Bỏ qua nếu TIMx null
    if (TIMx == NULL_PTR) return;

    // Tắt ngắt Capture/Compare và Update
    uint16 channelITFlag = TIM_IT_CC1 << (ch->channel - 1);
    TIM_ITConfig(TIMx, channelITFlag | TIM_IT_Update, DISABLE);
    notificationStates[ChannelNumber] = 0;
}

/* @brief: Hàm bật thông báo ngắt cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] Notification: Loại cạnh thông báo (PWM_RISING_EDGE, PWM_FALLING_EDGE, PWM_BOTH_EDGES)
 * @details: Bật thông báo cạnh cho kênh cụ thể
 */
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification){
    // Kiểm tra pwmConfig và ChannelNumber hợp lệ
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;

    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    // Bỏ qua nếu TIMx NULL hoặc ngắt đã bật
    if (TIMx == NULL_PTR) return;
    if(notificationStates[ChannelNumber]) return;

    // Bật ngắt Capture/Compare hoặc Update
    uint16 channelITFlag = TIM_IT_CC1 << (ch->channel - 1);
    if (Notification & PWM_RISING_EDGE) TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
    if (Notification & PWM_FALLING_EDGE) TIM_ITConfig(TIMx, channelITFlag, ENABLE);

    // Cấu hình NVIC cho IRQ timer
    IRQn_Type irq = (TIMx == TIM2) ? TIM2_IRQn
                  : (TIMx == TIM3) ? TIM3_IRQn
                  : (TIMx == TIM4) ? TIM4_IRQn
                  : (IRQn_Type)0xFF;
    if (irq != (IRQn_Type)0xFF) {
        NVIC_InitTypeDef nvicInit;
        nvicInit.NVIC_IRQChannel = irq;
        nvicInit.NVIC_IRQChannelPreemptionPriority = 1;
        nvicInit.NVIC_IRQChannelSubPriority = 0;
        nvicInit.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvicInit);
    }
    notificationStates[ChannelNumber] = 1;
}

/* @brief: Hàm lấy thông tin phiên bản PWM driver
 * @param[out] versioninfo: Con trỏ đến cấu trúc chứa thông tin phiên bản
 * @details: Trả về phiên bản module
 */
void Pwm_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    versioninfo->moduleID = 0x01;
    versioninfo->vendorID = 0x01;
    versioninfo->sw_major_version = 0x01;
    versioninfo->sw_minor_version = 0x00;
    versioninfo->sw_patch_version = 0x00;
}