#ifndef __MODE_CONTROL_H
#define __MODE_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "defaults.h"

// Patient profile configuration
void ModeControl_SetPatientProfile(PatientProfile profile);
PatientProfile ModeControl_GetPatientProfile(void);

// Flow calculation from differential pressure
float Flow_From_DiffPressure(float deltaP_cmH2O, PatientProfile profile);

// Ventilation mode handlers
void Mode_VCV(float targetVolume);
void Mode_SIMV(float targetVolume);
void Mode_PSV(float assistPressure);  // Added for full PSV support

#ifdef __cplusplus
}
#endif

#endif /* __MODE_CONTROL_H */
