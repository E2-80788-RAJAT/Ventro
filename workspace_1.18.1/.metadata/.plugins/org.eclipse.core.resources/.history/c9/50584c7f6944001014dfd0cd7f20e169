#ifndef __ALARM_H
#define __ALARM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Alarm bit flags structure
typedef union {
    struct {
        uint8_t overPressure   : 1;
        uint8_t underPressure  : 1;
        uint8_t noFlow         : 1;
        uint8_t reserved       : 5;
    } bits;
    uint8_t all;
} AlarmFlags;

void Alarm_Init(void);
void Alarm_Check(void);
AlarmFlags Alarm_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_H */
