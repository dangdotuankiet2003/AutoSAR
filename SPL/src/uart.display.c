#include "uart_display.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

// Hàm chuyển uint32_t thành chuỗi ASCII
static void uint32_to_string(uint32_t value, char *buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    uint8_t i = 0;
    char temp[10];
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }
    uint8_t j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

// @brief: Khởi tạo UART1 (PA9-TX, PA10-RX)
void UART_Display_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // Bật clock UART1

    USART_InitTypeDef usartInit;
    usartInit.USART_BaudRate = 115200;
    usartInit.USART_WordLength = USART_WordLength_8b;
    usartInit.USART_StopBits = USART_StopBits_1;
    usartInit.USART_Parity = USART_Parity_No;
    usartInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &usartInit);

    USART_Cmd(USART1, ENABLE);  // Bật UART1

    // Gửi tin nhắn khởi tạo
    const char *msg = "UART1 Initialized\r\n";
    for (uint8_t i = 0; msg[i] != '\0'; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, msg[i]);
    }
}

// @brief: Gửi số uint32_t qua UART1
void UART_Display_Send(uint32_t value) {
    char buffer[20];
    uint32_to_string(value, buffer);
    const char *prefix = "Value: ";
    for (uint8_t i = 0; prefix[i] != '\0'; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, prefix[i]);
    }
    for (uint8_t i = 0; buffer[i] != '\0'; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, buffer[i]);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, '\r');
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, '\n');
}