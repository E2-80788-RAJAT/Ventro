#ifndef __SENSOR_IF_H
#define __SENSOR_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define ABS_PRESSURE_ADDR 0x78
#define DIFF_PRESSURE_ADDR 0x79
#define FLOW_SENSOR_ADDR 0x40

typedef struct {
    float aPressure;   // Absolute pressure in cmH2O
    float dPressure;   // Differential pressure in cmH2O
    float flowRate;    // Flow rate in slm
    uint32_t timestamp;
} SensorData;

void Sensor_Init(void);
void Sensor_ReadAll(void);
SensorData Sensor_GetLatest(void);
bool Sensor_IsSpontaneousBreath(SensorData data);

float Sensor_ReadAMS5812(uint8_t address);
float Sensor_ReadSFM3300(uint8_t address);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_IF_H */
