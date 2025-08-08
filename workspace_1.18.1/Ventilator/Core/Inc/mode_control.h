
/*
 * mode_control.h
 *
 *  Created on: Jun 9, 2025
 *      Author: Admin
 */

#ifndef INC_MODE_CONTROL_H_
#define INC_MODE_CONTROL_H_
#include "defaults.h"


void DecidePatient(PatientProfile profile);
void ModeControl_SetPatientProfile(PatientProfile profile);
PatientProfile ModeControl_GetPatientProfile(void);

// Flow calculation from differential pressure
float Flow_From_DiffPressure(float deltaP_cmH2O, PatientProfile profile);

// Ventilation mode handlers
void Mode_VCV(float targetVolume);
void Mode_PCV(float targetPressure);

#endif /* INC_MODE_CONTROL_H_ */
