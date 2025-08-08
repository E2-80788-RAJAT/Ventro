#include "uart_comm.h"
#include "stm32f4xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef huart2; // Assumes UART2 connected to DWIN

void UART_Init(void)
{
    // Initialized via STM32CubeMX
}

void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

char UART_ReceiveChar(void)
{
    uint8_t ch;
    if (HAL_UART_Receive(&huart2, &ch, 1, 10) == HAL_OK)
        return (char)ch;
    return 0;
}
