#include "sensor_if.h"
#include "i2c_comm.h"
#include <math.h>

static SensorData latestSensorData;
static float prevFlow = 0.0f;
static float prevPressure = 0.0f;

void Sensor_Init(void)
{
    I2C_Init();
}

void Sensor_ReadAll(void)
{
    latestSensorData.aPressure = Sensor_ReadAMS5812(ABS_PRESSURE_ADDR);
    latestSensorData.dPressure = Sensor_ReadAMS5812(DIFF_PRESSURE_ADDR);
    latestSensorData.flowRate = Sensor_ReadSFM3300(FLOW_SENSOR_ADDR);
    latestSensorData.timestamp = HAL_GetTick();
}

SensorData Sensor_GetLatest(void)
{
    return latestSensorData;
}

bool Sensor_IsSpontaneousBreath(SensorData data)
{
    // Enhancement: Use derivative-based trigger detection
    float flowSlope = data.flowRate - prevFlow;
    float pressureSlope = data.aPressure - prevPressure;

    // Detection based on threshold OR dynamic change
    bool trigger = (
        data.flowRate > 2.0f ||
        data.aPressure < -2.0f ||
        flowSlope > 1.0f ||
        pressureSlope < -1.0f
    );

    // Update history for next cycle
    prevFlow = data.flowRate;
    prevPressure = data.aPressure;

    return trigger;
}

float Sensor_ReadAMS5812(uint8_t address)
{
    uint8_t buffer[4];
    if (I2C_Read(address, buffer, 4) != 0) return 0.0f;

    uint16_t raw_press = ((buffer[0] & 0x7F) << 8) | buffer[1];
    float psi = ((float)(raw_press - 3277) * 1.5f) / 26214.0f;
    float cmH2O = psi * 70.307f;
    return cmH2O;
}

float Sensor_ReadSFM3300(uint8_t address)
{
    uint8_t buffer[2];
    if (I2C_Read(address, buffer, 2) != 0) return 0.0f;

    uint16_t raw_flow = (buffer[0] << 8) | buffer[1];
    float flow_slm = ((float)raw_flow - 32768.0f) / 120.0f;
    return flow_slm;
}
