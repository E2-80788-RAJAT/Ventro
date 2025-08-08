/*
 * rrtos.h
 *
 *  Created on: Jun 13, 2025
 *      Author: Admin
 */

#ifndef SRC_RRTOS_H_
#define SRC_RRTOS_H_


#include <stdint.h>
typedef struct {
	void *sp;
	uint32_t timeout;
	uint8_t prio;

} OSThread;

typedef void (*OSThreadHandler)();

void OS_init(void *stkSto, uint32_t stkSize);
void OS_onIdle(void);
void OS_sched(void);
void OS_run(void);
void OS_delay(uint32_t ticks);
void OS_tick(void);
void OS_onStartup(void);

void OSThread_start(OSThread *me, uint8_t prio, OSThreadHandler threadHandler, void *stksto, uint32_t stkSize);

#endif /* SRC_RRTOS_H_ */
