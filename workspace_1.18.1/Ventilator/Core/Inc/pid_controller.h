/*
 * pid_controller.h
 *
 *  Created on: Jun 9, 2025
 *      Author: Admin
 */

#ifndef INC_PID_CONTROLLER_H_
#define INC_PID_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float Kp;
	float Ki;
	float Kd;
	float integral;
	float prev_error;
	float output;
} PIDController;

void PID_Init(PIDController *pid, float kp, float ki, float kd);
float PID_Compute(PIDController *pid, float setpoint, float measured, float dt);

#ifdef __cplusplus
}
#endif

#endif /* INC_PID_CONTROLLER_H_ */
