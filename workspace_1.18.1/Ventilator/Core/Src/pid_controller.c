/*
 * pid_controller.c
 *
 *  Created on: Jun 9, 2025
 *      Author: Admin
 */


#include "pid_controller.h"

void PID_Init(PIDController *pid, float kp, float ki, float kd)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

float PID_Compute(PIDController *pid, float setpoint, float measured, float dt)
{
    float error = setpoint - measured;
    pid->integral += error * dt;
    float derivative = (error - pid->prev_error) / dt;

    pid->output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
    pid->prev_error = error;

    return pid->output;
}
