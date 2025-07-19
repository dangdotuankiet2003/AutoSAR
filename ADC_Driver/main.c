#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "Port.h"
#include "Adc.h"
#include "Adc_Cfg.h"

// Cấu hình Port
static const Port_PinConfigType PortCfg_Plus[] = {
    {
        .PortNum = PORT_A,
        .PinNum = 0, // PA0 cho ADC
        .PinMode = PORT_PIN_MODE_ADC,
        .PinDirection = PORT_PIN_IN,
        .PinPull = 0, // No pull
        .PinSpeed = 0, // Low speed
        .PinDirectionChangeable = 0,
        .PinLevel = PORT_PIN_LEVEL_LOW,
        .ModeChangeable = 0
    },
    {
        .PortNum = PORT_A,
        .PinNum = 9, // PA9 cho UART TX
        .PinMode = PORT_PIN_MODE_UART,
        .PinDirection = PORT_PIN_OUT,
        .PinPull = 0, // No pull
        .PinSpeed = 2, // High speed
        .PinDirectionChangeable = 0,
        .PinLevel = PORT_PIN_LEVEL_LOW,
        .ModeChangeable = 0
    },
    {
        .PortNum = PORT_A,
        .PinNum = 10, // PA10 cho UART RX
        .PinMode = PORT_PIN_MODE_UART,
        .PinDirection = PORT_PIN_IN,
        .PinPull = 1, // Pull-up
        .PinSpeed = 2, // High speed
        .PinDirectionChangeable = 0,
        .PinLevel = PORT_PIN_LEVEL_LOW,
        .ModeChangeable = 0
    }
};

static const Port_ConfigType PortConfig = {
    .PinConfigs = PortCfg_Plus,
    .PinCount = 3
};

// Cấu hình ADC (chuyển sang oneshot)
Adc_ConfigType Adc_Configs[ADC_NUM_GROUPS] = {
    {
        .AdcInstance = ADC_INSTANCE_1,
        .ConversionMode = ADC_CONV_MODE_ONESHOT,
        .TriggerSource = ADC_TRIGGER_SOFTWARE,
        .NumChannels = 1,
        .NotificationEnabled = ADC_NOTIFICATION_ENABLE,
        .Initialback = NULL_PTR, // Sẽ gán trong main
        .Channels = {{.ChannelId = 0, .SampleTime = ADC_SampleTime_28Cycles5, .Rank = 1}}
    }
};

Adc_GroupDefType Adc_GroupConfigs[ADC_MAX_GROUPS] = {
    {
        .id = 0,
        .AdcInstance = ADC_INSTANCE_1,
        .Channels = {0},
        .Priority = 0,
        .NumChannels = 1,
        .Status = ADC_IDLE,
        .Result = NULL_PTR, 
        .Adc_StreamEnableType = 1,
        .Adc_StreamBufferSize = 1,
        .Adc_StreamBufferMode = ADC_STREAM_BUFFER_CIRCULAR
    }
};

// Bộ đệm và biến trạng thái
static Adc_ValueGroupType adct_groupa_buffer[1];
static volatile uint8 conversionComplete = 0;

// Callback cho ngắt ADC
void Adc_Notification_Callback(void) {
    conversionComplete = 1;
}

// Hàm gửi ký tự qua UART
void UART_SendChar(USART_TypeDef* USARTx, char c) {
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(USARTx, c);
}

// Hàm gửi số qua UART
void UART_SendNumber(USART_TypeDef* USARTx, uint16_t value) {
    char temp[6];
    int i = 0;
    if (value == 0) {
        UART_SendChar(USARTx, '0');
        return;
    }
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }
    while (i > 0) {
        UART_SendChar(USARTx, temp[--i]);
    }
}

// Hàm gửi chuỗi qua UART
void UART_SendString(USART_TypeDef* USARTx, const char* str) {
    while (*str) {
        UART_SendChar(USARTx, *str++);
    }
}

// Hàm gửi trạng thái nhóm ADC qua UART
void UART_SendGroupStatus(USART_TypeDef* USARTx, Adc_StatusType status) {
    UART_SendString(USARTx, "Status: ");
    switch (status) {
        case ADC_IDLE: UART_SendString(USARTx, "IDLE"); break;
        case ADC_BUSY: UART_SendString(USARTx, "BUSY"); break;
        case ADC_COMPLETED: UART_SendString(USARTx, "COMPLETED"); break;
        case ADC_STREAM_COMPLETED: UART_SendString(USARTx, "STREAM_COMPLETED"); break;
        default: UART_SendString(USARTx, "UNKNOWN"); break;
    }
    UART_SendString(USARTx, "\r\n");
}

// Hàm delay đơn giản (cho clock 8MHz)
void Delay(uint32_t ms) {
    uint32_t i;
    for (i = 0; i < ms * 8000; i++);
}

int main(void) {
    // Cấu hình clock hệ thống (8MHz HSE)
    RCC_HSEConfig(RCC_HSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
    RCC_HCLKConfig(RCC_SYSCLK_Div1); // HCLK = 8MHz
    RCC_PCLK2Config(RCC_HCLK_Div1); // PCLK2 = 8MHz
    RCC_PCLK1Config(RCC_HCLK_Div1); // PCLK1 = 8MHz (cho DMA)

    // Khởi tạo NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // Khởi tạo Port
    Port_Init(&PortConfig);

    // Khởi tạo UART1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    // Gán callback và buffer cho ADC
    Adc_Configs[0].Initialback = Adc_Notification_Callback;
    Adc_GroupConfigs[0].Result = adct_groupa_buffer;

    // Khởi tạo ADC
    Adc_Init(Adc_Configs);
    UART_SendString(USART1, "ADC Initialized\r\n");

    if (Adc_SetupResultBuffer(0, adct_groupa_buffer) == E_OK) {
        UART_SendString(USART1, "Result Buffer Setup OK\r\n");
    } else {
        UART_SendString(USART1, "Result Buffer Setup Failed\r\n");
    }

    Adc_EnableGroupNotification(0);
    UART_SendString(USART1, "Notification Enabled\r\n");

    Adc_EnableDma(0);
    UART_SendString(USART1, "DMA Enabled\r\n");

    Adc_ValueGroupType adcValue;

    while (1) {
        // Tắt ADC và DMA trước khi bắt đầu chuyển đổi mới
        Adc_DisableGroupNotification(0);
        Adc_DisableDma(0);
        Adc_EnableGroupNotification(0);
        Adc_EnableDma(0);

        conversionComplete = 0;
        Adc_StartGroupConversion(0);
        UART_SendString(USART1, "Conversion Started\r\n");

        uint32_t timeout = 30000000; // ~3.75s tại 8MHz
        while (!conversionComplete && timeout > 0) {
            timeout--;
        }

        if (timeout == 0) {
            UART_SendString(USART1, "Error: Timeout\r\n");
            UART_SendGroupStatus(USART1, Adc_GetGroupStatus(0));
            UART_SendString(USART1, "ADC Flags: STRT=");
            UART_SendChar(USART1, ADC_GetFlagStatus(ADC1, ADC_FLAG_STRT) ? '1' : '0');
            UART_SendString(USART1, ", EOC=");
            UART_SendChar(USART1, ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) ? '1' : '0');
            UART_SendString(USART1, ", DR=");
            UART_SendNumber(USART1, ADC1->DR);
            UART_SendString(USART1, ", DMA_TC=");
            UART_SendChar(USART1, DMA_GetFlagStatus(DMA1_FLAG_TC1) ? '1' : '0');
            UART_SendString(USART1, "\r\n");
        } else if (Adc_ReadGroup(0, &adcValue) == E_OK) {
            UART_SendString(USART1, "ADC: ");
            UART_SendNumber(USART1, adcValue);
            UART_SendString(USART1, "\r\n");
        } else {
            UART_SendString(USART1, "Error: ReadGroup Failed\r\n");
            UART_SendGroupStatus(USART1, Adc_GetGroupStatus(0));
        }

        Delay(500);
    }
}