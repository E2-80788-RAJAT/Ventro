#include "utils.h"
#include <string.h>

void Delay_ms(uint32_t ms)
{
    HAL_Delay(ms);  // Blocking delay
}

void Mem_Clear(void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

uint16_t ClampU16(uint16_t val, uint16_t min, uint16_t max)
{
    if (min > max) return val; // invalid range, do nothing
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

float ClampF(float val, float min, float max)
{
    if (min > max) return val;
    if (val < min) return min;
    if (val > max) return max;
    return val;
}
