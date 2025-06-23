#include "Dio.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

void Delay(uint32_t nCount)
{
    for (; nCount != 0; nCount--); // Giảm nCount về 0 để tạo độ trễ
}

int main(void)
{
    SystemInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    while (1)
    {
        Dio_WriteChannel(DIO_CHANNEL_C13, STD_LOW);
        Delay(8000000); 

        Dio_WriteChannel(DIO_CHANNEL_C13, STD_HIGH);
        Delay(8000000); 

        //   Dio_FlipChannel(DIO_CHANNEL_C13); 
        //   Delay_Ms(1000);
    }
}



