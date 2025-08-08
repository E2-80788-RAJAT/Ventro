#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"

void Delay_ms(uint32_t ms);
void Mem_Clear(void *ptr, size_t size);
uint16_t ClampU16(uint16_t val, uint16_t min, uint16_t max);
float ClampF(float val, float min, float max);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_H */
