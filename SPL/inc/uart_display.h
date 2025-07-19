#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f10x.h"

// @brief: Khởi tạo UART1 (PA9-TX, PA10-RX)
// @details: Config UART1 với baud 115200, 8-bit, 1 stop, no parity
void UART_Display_Init(void);

// @brief: Gửi số uint32_t qua UART1
// @param[in] value: Giá trị số cần gửi
// @details: Chuyển số thành chuỗi ASCII và gửi qua UART1 với format "Value: <số>\r\n"
void UART_Display_Send(uint32_t value);

#endif /* UART_DISPLAY_H */