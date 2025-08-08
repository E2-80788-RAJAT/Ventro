#include "i2c_comm.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1; // Assumes I2C1 used for sensors

void I2C_Init(void)
{
    // Initialization handled by STM32CubeMX-generated code
}

int I2C_Read(uint8_t devAddr, uint8_t *pData, uint16_t size)
{
    if (HAL_I2C_Master_Receive(&hi2c1, (devAddr << 1), pData, size, 100) == HAL_OK)
        return 0;
    return -1;
}

int I2C_Write(uint8_t devAddr, uint8_t *pData, uint16_t size)
{
    if (HAL_I2C_Master_Transmit(&hi2c1, (devAddr << 1), pData, size, 100) == HAL_OK)
        return 0;
    return -1;
}
