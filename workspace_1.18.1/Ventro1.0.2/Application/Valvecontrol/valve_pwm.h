#ifndef __VALVE_PWM_H
#define __VALVE_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void ValvePWM_Init(void);
void ValvePWM_SetDuty(uint8_t duty);
float ValvePWM_ControlVolume(float targetVol, float currentVol);

#ifdef __cplusplus
}
#endif

#endif /* __VALVE_PWM_H */
