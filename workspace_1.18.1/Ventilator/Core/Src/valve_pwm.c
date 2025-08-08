/*
 * valve_pwm.c
 *
 *  Created on: Jun 9, 2025
 *      Author: Admin
 */

#include "valve_pwm.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include "stdint.h"
#include "pid_controller.h"

#define PWM_TIMER       htim1
#define PWM_CHANNEL     TIM_CHANNEL_1
#define PWM_MIN_DUTY    50
#define PWM_MAX_DUTY    120
#define RAMP_STEP       10.0f
#define RAMP_MAX_DUTY   400.0f

extern TIM_HandleTypeDef PWM_TIMER;

static float currentRampDuty = 0.0f;
static PIDController volumePID;

void ValvePWM_Init(void)
{
	HAL_TIM_PWM_Start(&PWM_TIMER, PWM_CHANNEL);
	ValvePWM_SetDuty(75);
	currentRampDuty = 0.0f;
}

void ValvePWM_InitPID(float kp, float ki, float kd) {
    PID_Init(&volumePID, kp, ki, kd);
}

void ValvePWM_SetDuty(uint8_t duty)
{
	if (duty < PWM_MIN_DUTY) duty = PWM_MIN_DUTY;
	if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
	__HAL_TIM_SET_COMPARE(&PWM_TIMER, PWM_CHANNEL, duty);
}

float ValvePWM_ControlVolume(float targetVol, float currentVol)
{
    float dt = 0.01f;
    float output = PID_Compute(&volumePID, targetVol, currentVol, dt);
    if (output > PWM_MAX_DUTY) output = PWM_MAX_DUTY;
    else if (output < PWM_MIN_DUTY) output = PWM_MIN_DUTY;
    return output;
}

float ValvePWM_ControlPressure(float targetPressure, float currentPressure)
{
	float error = targetPressure - currentPressure;
	float gain = 3.0f;
	float duty = 80.0f + gain * error;

	if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
	else if (duty < PWM_MIN_DUTY) duty = PWM_MIN_DUTY;

	return duty;
}

float ValvePWM_RampUp(float targetDuty)
{
	if (currentRampDuty < targetDuty) {
		currentRampDuty += RAMP_STEP;
		if (currentRampDuty > targetDuty)
			currentRampDuty = targetDuty;
	}
	ValvePWM_SetDuty((uint8_t)currentRampDuty);
	return currentRampDuty;
}

void ValvePWM_ResetRamp(void)
{
	currentRampDuty = 0.0f;
}
