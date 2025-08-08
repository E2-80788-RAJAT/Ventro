/*
 * Atmega.h
 *
 *  Created on: Jun 9, 2025
 *      Author: Admin
 */

#ifndef INC_ATMEGA_H_
#define INC_ATMEGA_H_
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern uint8_t final_sensor_data[12];
extern uint8_t sensor_data[24];

//extern int16_t pressure_adc = 0;
//extern uint8_t update_flag = 0;
//extern uint8_t storevalue = 0;
//extern uint8_t storevalue1 = 0;
//extern uint16_t flow_adc = 0;
//extern float pressure_data = 0.0;
//extern float flow_data = 0.0;
//extern float flow = 0.0;
//extern float mbar = 0.0;
//extern float flow = 0.0;
//extern int pressure = 0;



void ATmega_Receive(void);
#endif /* INC_ATMEGA_H_ */
