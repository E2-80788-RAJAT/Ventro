#ifndef __I2C_COMM_H
#define __I2C_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void I2C_Init(void);
int I2C_Read(uint8_t devAddr, uint8_t *pData, uint16_t size);
int I2C_Write(uint8_t devAddr, uint8_t *pData, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_COMM_H */
