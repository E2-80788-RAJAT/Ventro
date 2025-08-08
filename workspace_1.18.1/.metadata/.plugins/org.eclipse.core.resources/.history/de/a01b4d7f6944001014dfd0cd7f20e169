#ifndef __DEFAULTS_H
#define __DEFAULTS_H

#ifdef __cplusplus
extern "C" {
#endif

// Default parameters for ventilation modes
#define DEFAULT_VCV_TIDAL_VOL     500.0f // mL
#define DEFAULT_PCV_PRESSURE      20.0f  // cmH2O
#define DEFAULT_SIMV_RATE         12     // breaths/min
#define DEFAULT_PSV_TRIGGER       2.0f   // L/min

// Sensor I2C addresses
#define ABS_PRESSURE_ADDR         0x78
#define DIFF_PRESSURE_ADDR        0x79
#define FLOW_SENSOR_ADDR          0x40

// Alarm limits
#define PRESSURE_HIGH_LIMIT       40.0f  // cmH2O
#define PRESSURE_LOW_LIMIT        5.0f   // cmH2O
#define NO_FLOW_THRESHOLD         0.1f   // slm

// Patient Profiles
typedef enum {
    PROFILE_ADULT = 0,
    PROFILE_CHILD
} PatientProfile;

#define DEFAULT_PATIENT_PROFILE PROFILE_ADULT

#ifdef __cplusplus
}
#endif

#endif /* __DEFAULTS_H */
