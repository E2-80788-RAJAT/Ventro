#ifndef __BATTERY_MGR_H
#define __BATTERY_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POWER_UNKNOWN = 0,
    POWER_MAINS,
    POWER_BATTERY
} PowerSource;

void Battery_Init(void);
void Battery_UpdateStatus(void);
float Battery_ReadVBAT(void);
PowerSource Battery_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* __BATTERY_MGR_H */
