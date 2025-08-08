#include "alarm.h"
#include "stm32f4xx_hal.h"
#include "sensor_if.h"
#include <math.h>

#define ALARM_BUZZER_PIN     GPIO_PIN_1
#define ALARM_BUZZER_PORT    GPIOB
#define PRESSURE_HIGH_LIMIT  40.0f   // cmH2O
#define PRESSURE_LOW_LIMIT   5.0f    // cmH2O
#define NO_FLOW_THRESHOLD    0.1f    // slm

static AlarmFlags currentAlarms;

void Alarm_Init(void)
{
    HAL_GPIO_WritePin(ALARM_BUZZER_PORT, ALARM_BUZZER_PIN, GPIO_PIN_RESET);
    currentAlarms.all = 0;
}

void Alarm_Check(void)
{
    SensorData data = Sensor_GetLatest();
    currentAlarms.all = 0;

    if (data.aPressure > PRESSURE_HIGH_LIMIT)
        currentAlarms.bits.overPressure = 1;

    if (data.aPressure < PRESSURE_LOW_LIMIT)
        currentAlarms.bits.underPressure = 1;

    if (fabs(data.flowRate) < NO_FLOW_THRESHOLD)
        currentAlarms.bits.noFlow = 1;

    if (currentAlarms.all != 0)
        HAL_GPIO_WritePin(ALARM_BUZZER_PORT, ALARM_BUZZER_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(ALARM_BUZZER_PORT, ALARM_BUZZER_PIN, GPIO_PIN_RESET);
}

AlarmFlags Alarm_GetStatus(void)
{
    return currentAlarms;
}
