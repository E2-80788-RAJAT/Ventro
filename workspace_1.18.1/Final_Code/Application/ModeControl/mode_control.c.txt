#include "mode_control.h"
#include "sensor_if.h"
#include "valve_pwm.h"
#include "defaults.h"
#include <math.h>

static PatientProfile currentPatientProfile = DEFAULT_PATIENT_PROFILE;
static float volumeAccumulator = 0.0f;
static float samplePeriod_ms = 10.0f;
static int breathCycleTimer_ms = 0;
static int breathInterval_ms = 4000; // ~15 bpm

void ModeControl_SetPatientProfile(PatientProfile profile)
{
    currentPatientProfile = profile;
}

PatientProfile ModeControl_GetPatientProfile(void)
{
    return currentPatientProfile;
}

float Flow_From_DiffPressure(float deltaP_cmH2O, PatientProfile profile)
{
    float K = (profile == PROFILE_CHILD) ? 0.15f : 0.25f;
    float flow = K * sqrtf(fabsf(deltaP_cmH2O));
    return (deltaP_cmH2O >= 0) ? flow : -flow;
}

void Mode_VCV(float targetVolume)
{
    SensorData sensor = Sensor_GetLatest();
    float flow = Flow_From_DiffPressure(sensor.dPressure, currentPatientProfile);
    float tidalVolumeIncrement = flow * (samplePeriod_ms / 60000.0f); // L
    volumeAccumulator += tidalVolumeIncrement;

    float duty = ValvePWM_ControlVolume(targetVolume, volumeAccumulator);
    ValvePWM_SetDuty(duty);
}

void Mode_SIMV(float targetVolume)
{
    static int phase = 0; // 0 = idle, 1 = inspiration, 2 = expiration
    SensorData sensor = Sensor_GetLatest();

    float flow = Flow_From_DiffPressure(sensor.dPressure, currentPatientProfile);
    float tidalVolumeIncrement = flow * (samplePeriod_ms / 60000.0f); // L
    volumeAccumulator += tidalVolumeIncrement;

    bool spontaneousTrigger = (flow > 2.0f || sensor.aPressure < -2.0f);

    if (phase == 0) {
        if (spontaneousTrigger || breathCycleTimer_ms >= breathInterval_ms) {
            phase = 1;
            breathCycleTimer_ms = 0;
        }
    }

    if (phase == 1) {
        float duty = ValvePWM_ControlVolume(targetVolume, volumeAccumulator);
        ValvePWM_SetDuty(duty);

        if (volumeAccumulator >= targetVolume) {
            phase = 2;
            ValvePWM_SetDuty(0); // End inspiration
        }
    }

    if (phase == 2) {
        // Passive expiration
        volumeAccumulator = 0.0f;
        phase = 0;
    }

    breathCycleTimer_ms += (int)samplePeriod_ms;
}

void Mode_PSV(float assistPressure)
{
    static int phase = 0; // 0 = idle, 1 = assist/ramp, 2 = expiration
    static float peakFlow = 0.0f;

    SensorData sensor = Sensor_GetLatest();
    float flow = Flow_From_DiffPressure(sensor.dPressure, currentPatientProfile);

    bool trigger = (flow > 2.0f || sensor.aPressure < -2.0f);
    bool terminate = (flow < 0.25f * peakFlow && peakFlow > 0.0f);

    if (phase == 0 && trigger) {
        phase = 1;
        peakFlow = flow;
    }

    if (phase == 1) {
        if (flow > peakFlow) peakFlow = flow;

        float duty = ValvePWM_ControlPressure(assistPressure, sensor.aPressure);
        ValvePWM_SetDuty(duty);

        if (terminate) {
            ValvePWM_SetDuty(0);
            phase = 2;
        }
    }

    if (phase == 2) {
        // Expiration
        peakFlow = 0.0f;
        phase = 0;
    }
}
