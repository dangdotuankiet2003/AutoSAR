#include <stdio.h>
#include "Std_Types.h"
#include "Pwm.h"
#include "Pwm_Cfg.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
/* Biến nội bộ để lưu cấu hình sau khi khởi tạo*/
static const Pwm_ConfigType* pwmConfig = NULL_PTR;

/* Macro để lưu trạng thái notification cho từng kênh */
static uint8_t notificationStates[PWM_NUM_CHANNELS] = {0};

/* @brief: Hàm khởi tạo PWM driver
 * @param[in] ConfigPtr: Con trỏ đến cấu hình PWM
 * @details: Khởi tạo biến nội bộ và phần cứng PWM theo cấu hình
 */
void Pwm_Init(const Pwm_ConfigType* ConfigPtr){
    pwmConfig = ConfigPtr;

    /* Duyệt qua từng kênh để khởi tạo cấu hình cho từng kênh */
    for(uint8 i = 0; i < pwmConfig->numChannels; i++){
        const Pwm_ChannelConfigType *ch = &pwmConfig->channels[i];
        TIM_TypeDef *TIMx = ch->TIMx;

        /* CNT = 0	Sườn lên (rising edge) - CNT = CCR Sườn xuống (falling edge*/
        /* Chọn chế độ Output Compare dựa vào polarity - Thanh ghi CCR:
         * - PWM1: Counter < CCR → Active   (HIGH nếu polarity=High và ngược lại)
         * - PWM2: Counter < CCR → Inactive (LOW nếu polarity=High và ngược lại) 
         */
        uint16 ocMode = (ch->polarity == PWM_HIGH) ? TIM_OCMode_PWM1 : TIM_OCMode_PWM2;
        
        /* Bật clock cho Timer */
        if(TIMx == TIM2)        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        else if(TIMx == TIM3)   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        else if(TIMx == TIM4)   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
        /* Cấu hình Time Base */
        TIM_TimeBaseInitTypeDef timBase;
        timBase.TIM_Prescaler = ch->prescaler;
        timBase.TIM_Period = ch->defaultPeriod - 1;      /* ARR = period - 1 */
        timBase.TIM_ClockDivision = 0;                   /* Không chia tần số */
        timBase.TIM_CounterMode = TIM_CounterMode_Up;    /* Đếm lên */
        TIM_TimeBaseInit(TIMx, &timBase);

        /*   Cấu hình Output Compare để quyết định duty cycle, với các thông số:
         * - OCMode: Chế độ PWM1/PWM2
         * - OutputState: Bật đầu ra
         * - Pulse (CCR): Giá trị so sánh (quyết định duty cycle)
         * - OCPolarity: Cực tính đầu ra
         */
        TIM_OCInitTypeDef ocInitStructure;
        ocInitStructure.TIM_OCMode = ocMode;            /* Chọn Mode PWM1 | PWM2 */
        ocInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        ocInitStructure.TIM_Pulse = (ch->defaultDutyCycle * timBase.TIM_Period) / 0x8000; /* Độ rông xung = CCR */
        ocInitStructure.TIM_OCPolarity = (ch->polarity == PWM_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low;
    
        /* Khởi tạo các kênh theo ID */
        if(ch->channel == 1) TIM_OC1Init(TIMx, &ocInitStructure);
        else if(ch->channel == 2) TIM_OC2Init(TIMx, &ocInitStructure);
        else if(ch->channel == 3) TIM_OC3Init(TIMx, &ocInitStructure);
        else if(ch->channel == 4) TIM_OC4Init(TIMx, &ocInitStructure);

        /* Bật Timer */
        TIM_Cmd(TIMx, ENABLE);

        /* Bật ngắt CCx và Update ngay nếu notificationEnable = 1 từ config */
        if(ch->notificationEnable){
            uint16 channelITFlag = TIM_IT_CC1 << (ch->channel - 1);
            TIM_ITConfig(TIMx, channelITFlag | TIM_IT_Update, ENABLE);  /* Bật ngắt CCx và Update */
        }

        /* Kiểm tra đã bật thông báo cgo PWM từ config hay chưa */
        if(ch->notificationEnable) Pwm_EnableNotification(i, PWM_BOTH_EDGES);

        /* Lưu trạng thái thông báo ngắt của PWM */
        notificationStates[i] = ch->notificationEnable;
    }  
}

/* @brief: Hàm hủy khởi tạo PWM driver
 * @details: Đặt PWM về trạng thái ban đầu, tắt ngắt và thông báo
 */
void Pwm_DeInit(void){
    /* Nếu chưa khởi tạo cấu hình cho TIM */
    if(pwmConfig == NULL_PTR) return;

    /* Duyệt qua từng kênh để hủy khởi tạo TIM của các kênh */
    for(uint8 i = 0; i < pwmConfig->numChannels; i++){
        const Pwm_ChannelConfigType *ch = &pwmConfig->channels[i];

        TIM_TypeDef *TIMx = ch->TIMx;

        /* Tắt Timer*/
        TIM_Cmd(TIMx, DISABLE);

        /* Tắt Thông báo ngắt của các kênh nếu đang bật */
        if(notificationStates[i]){
            Pwm_DisableNotification(i);
            notificationStates[i] = 0; /* Reset flag - Tắt thông báo*/
        }
        /* Tắt clock cho Timer */
        if (TIMx == TIM2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
        else if (TIMx == TIM3) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
        else if (TIMx == TIM4) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, DISABLE);
    }

    /* Reset flag báo rằng không còn cấu hình TIM cho các kênh*/
    pwmConfig = NULL_PTR;
}

/* @brief: Hàm đặt duty cycle cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] DutyCycle: Giá trị duty cycle (0x0000: 0%, 0x8000: 100%)
 * @details: Cập nhật duty cycle mà không thay đổi chu kỳ
 */
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16_t DutyCycle){
    /* Kiểm tra nếu chưa init hoặc kênh không hợp lệ */
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;
    
    /* Duyệt tới channel của TIM */
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    /* Lấy giá trị ARR hiện tại để tính giá trị CCR mới */
    uint16 arr = TIMx->ARR; 
    /*Tính giá trị ccr */
    uint16 ccr = (DutyCycle * arr ) / 0x8000; /* CCR = (duty * ARR) / 0x8000 */

    /* Lưu giá trị ccr vào thanh ghi CCR tương ứng của kênh */
    if (ch->channel == 1)     TIM_SetCompare1(TIMx, ccr);
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
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16_t DutyCycle){
    /* Kiểm tra nếu chưa khởi tạo hoặc kênh không hợp lệ */
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;
    
    /* Duyệt tới channel của TIM */
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    /* kiểm tra xem có đang khởi tạo cho phép thay đổi chu kì & duty cycle không*/
    if(ch->channelClass != PWM_VARIABLE_PERIOD) return;

    /* Tính giá trị ARR mới (period - 1) và CCR dựa ARR mới */
    uint16 arr = Period - 1; /* ARR = period - 1 */
    uint16 ccr = (DutyCycle * arr) / 0x8000; /* CCR = (duty * ARR) / 0x8000 */
    /* Lưu/Set giá trị arr */
    TIM_SetAutoreload(TIMx, arr);

    /* Lưu giá trị ccr vào thanh ghi CCR tương ứng của kênh */
    if (ch->channel == 1) TIM_SetCompare1(TIMx, ccr);
    else if(ch->channel == 2) TIM_SetCompare2(TIMx, ccr);
    else if(ch->channel == 3) TIM_SetCompare3(TIMx, ccr);
    else if(ch->channel == 4) TIM_SetCompare4(TIMx, ccr);
}

/* @brief: Hàm đặt đầu ra PWM về trạng thái idle
 * @param[in] ChannelNumber: Số kênh PWM
 * @details: Đặt đầu ra về trạng thái idle đã cấu hình (high/low)
 */
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber){
    /* Kiểm tra nếu chưa init hoặc kênh không hợp lệ */
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;
    
    /* Duyệt tới channel của TIM */
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    /* Tắt output compare channel để dừng PWM (disable đầu ra PWM) */
    /* TIM_CCxCmd dùng để enable hoặc disable đầu ra của kênh Capture/Compare (CCx) trên timer */
    if (ch->channel == 1) TIM_CCxCmd(TIMx, TIM_Channel_1, TIM_CCx_Disable);
    else if (ch->channel == 2) TIM_CCxCmd(TIMx, TIM_Channel_2, TIM_CCx_Disable);
    else if (ch->channel == 3) TIM_CCxCmd(TIMx, TIM_Channel_3, TIM_CCx_Disable);
    else if (ch->channel == 4) TIM_CCxCmd(TIMx, TIM_Channel_4, TIM_CCx_Disable);

    GPIO_TypeDef *GPIOx = NULL_PTR;
    uint16 gpioPin = 0;

    /* Kiểm tra map pin của TIM */
    /* Đối với TIM2 */
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
    /* Đối với TIM3  */
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
    /* Đối với TIM4  */
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

    /* Nếu map thành công, set pin về idle state */
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
    /* Kiểm tra nếu chưa init hoặc kênh không hợp lệ */
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return PWM_LOW;
    
    /* Duyệt tới channel của TIM */
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    GPIO_TypeDef *GPIOx = NULL_PTR;
    uint16 gpioPin = 0;

    /* Kiểm tra map pin của TIM */
    /* Đối với TIM2  */
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
    /* Đối với TIM3  */
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
    /* Đối với TIM4 */
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
    if(GPIOx != NULL_PTR){
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
    /* Kiểm tra nếu chưa init hoặc kênh không hợp lệ */
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;

    /* Lưu con trỏ trở tới kênh PWM muốn tắt */
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    /* Tính IT flag cho kênh muốn tắt */
    uint16 channelITFlag = TIM_IT_CC1 << (ch->channel - 1);
    
    /* Tắt IT cho thanh ghi CCx và Update */
    TIM_ITConfig(TIMx, channelITFlag | TIM_IT_Update, DISABLE);
    /* Reset flag để báo rằng tắt thông báo cho PWM */
    notificationStates[ChannelNumber] = 0;
}

/* @brief: Hàm bật thông báo ngắt cho kênh PWM
 * @param[in] ChannelNumber: Số kênh PWM
 * @param[in] Notification: Loại cạnh thông báo (PWM_RISING_EDGE, PWM_FALLING_EDGE, PWM_BOTH_EDGES)
 * @details: Bật thông báo cạnh cho kênh cụ thể
 */

void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification){
    /* Kiểm tra nếu chưa init hoặc kênh không hợp lệ */
    if(pwmConfig == NULL_PTR || ChannelNumber >= pwmConfig->numChannels) return;

    /* Lưu con trỏ trở tới kênh PWM muốn bật */
    const Pwm_ChannelConfigType *ch = &pwmConfig->channels[ChannelNumber];
    TIM_TypeDef *TIMx = ch->TIMx;

    /* Kiểm tra đã bật thông báo trước đó hay chưa */
    if(notificationStates[ChannelNumber]) return;

    /* Tính IT flag cho kênh muốn bật thông báo */
    uint16 channelITFlag = TIM_IT_CC1 << (ch->channel - 1);

    /* Bật IT cho Update nếu có rising edge (start cycle, output HIGH in PWM1 HIGH polarity) */
    if (Notification & PWM_RISING_EDGE) TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);

    /* Bật IT cho CCx nếu có falling edge (match CCR, output LOW) */
    if (Notification & PWM_FALLING_EDGE) TIM_ITConfig(TIMx, channelITFlag, ENABLE);

    /* Cấu hình và bật NVIC cho IRQ timer */
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
    /* Cập nhật đã set flasg thông báo*/
    notificationStates[ChannelNumber] = 1;
}

/* @brief: Hàm lấy thông tin phiên bản PWM driver
 * @param[out] versioninfo: Con trỏ đến cấu trúc chứa thông tin phiên bản
 * @details: Trả về phiên bản module, ...
 */
void Pwm_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo == NULL_PTR) return;
    VersionInfo->moduleID = 0x01;
    VersionInfo->vendorID = 0x01;
    VersionInfo->sw_major_version = 0x01;
    VersionInfo->sw_minor_version = 0x00;
    VersionInfo->sw_patch_version = 0x00;
} 