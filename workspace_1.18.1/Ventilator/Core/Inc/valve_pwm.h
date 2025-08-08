/*
 * valve_pwm.h
 *
 *  Created on: Jun 9, 2025
 *      Author: Admin
 */

#ifndef INC_VALVE_PWM_H_
#define INC_VALVE_PWM_H_

#include "stdint.h"

void ValvePWM_Init(void);
void ValvePWM_SetDuty(uint8_t duty);
float ValvePWM_ControlVolume(float targetVol, float currentVol);
void ValvePWM_InitPID(float kp, float ki, float kd);
float ValvePWM_ControlPressure(float targetPressure, float currentPressure);
float ValvePWM_RampUp(float targetDuty);
void ValvePWM_ResetRamp(void);
#endif /* INC_VALVE_PWM_H_ */
