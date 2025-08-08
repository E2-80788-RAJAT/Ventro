/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdbool.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
/* USER CODE BEGIN PV */
char rx_buffer[9];
float get_pressure;
uint32_t adc_Value;
float voltage;
float pressure;
char msg[150];
char buf[100];
float pressPSI;
#define TOLERANCE 3 // Tolerance range for fluctuating values (±2g around the target)
uint32_t current_time;
uint32_t last_time_1;


#define FLASH_USER_START_ADDR ((uint32_t)0x080E0000) // Flash address to store data

#define HX711_DT_PIN GPIO_PIN_0
#define HX711_DT_PORT GPIOA
#define HX711_SCK_PIN GPIO_PIN_1
#define HX711_SCK_PORT GPIOA
char RxData[11];
#define NUM_READINGS 10
float average_weight = 0.0;
float weight_readings[NUM_READINGS];
float sum = 0.0;
float get_wieght = 0;

// Function prototypes
void Flash_WriteData(uint32_t address, void *data, uint32_t size);
void Flash_ReadData(uint32_t address, void *data, uint32_t size);
void HX711_Init(void);
int32_t HX711_Read(void);
float HX711_GetWeight(int32_t offset, float scale);

// Flash Write Function
void Flash_WriteData(uint32_t address, void *data, uint32_t size) {
	HAL_FLASH_Unlock();
	for (uint32_t i = 0; i < size; i += 4) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i,
				*(uint32_t*) ((uint8_t*) data + i));
	}
	HAL_FLASH_Lock();
}

// Flash Read Function
void Flash_ReadData(uint32_t address, void *data, uint32_t size) {
	memcpy(data, (void*) address, size);
}

char buffer1[100];
char buffer2[200];
char buffer3[200];
char buffer4[200];
char buffer0[100];
char buffer5[50];
char buffer6[100];
char buffer7[100];
char buffer[100] = { 0 };
//uint8_t rx_buffer[100];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void const *argument);
void StartTask02(void const *argument);

/* USER CODE BEGIN PFP */
//void WeightMeasurementTask(void *parameters);
//void ValveOperatingTask(void *parameters);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HX711_Init(void) {
	HAL_GPIO_WritePin(HX711_SCK_PORT, HX711_SCK_PIN, GPIO_PIN_RESET);
}

int32_t HX711_Read(void) {
	int32_t data = 0;
	uint8_t i;

	while (HAL_GPIO_ReadPin(HX711_DT_PORT, HX711_DT_PIN) == GPIO_PIN_SET)
		;

	for (i = 0; i < 24; i++) {
		HAL_GPIO_WritePin(HX711_SCK_PORT, HX711_SCK_PIN, GPIO_PIN_SET); // SCK
		//HAL_Delay(10);
		data = (data << 1) | HAL_GPIO_ReadPin(HX711_DT_PORT, HX711_DT_PIN);
		HAL_GPIO_WritePin(HX711_SCK_PORT, HX711_SCK_PIN, GPIO_PIN_RESET); // SCK
		//HAL_Delay(100);
	}

	HAL_GPIO_WritePin(HX711_SCK_PORT, HX711_SCK_PIN, GPIO_PIN_SET); // 25th pulse
	HAL_GPIO_WritePin(HX711_SCK_PORT, HX711_SCK_PIN, GPIO_PIN_RESET);

	if (data & 0x800000) {
		data |= 0xFF000000;
	}

	return data;
}
float HX711_GetWeight(int32_t offset, float scale) {
	int32_t raw_value = HX711_Read();
	float weight = ((float) (raw_value - offset) / scale);

	// Avoid displaying small negative values due to noise
	if (fabs(raw_value - offset) < 500) {
		return 0.0;
	}
	return weight;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
// Fun to send the data to LCD

void SendIconToDWIN(void) {
	uint8_t icon_command[] = { 0x5a, 0xa5, 0x05, 0x82, 0x12, 0x50, 0x00, 0x22}; // Example command, refer to your display's documentation for correct values.
	HAL_UART_Transmit(&huart2, icon_command, sizeof(icon_command), 500);
}

void Reset_GPIO_Pins1(void) {
	HAL_GPIO_WritePin(GPIOD,
			GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
					| GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
					| GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
					| GPIO_PIN_15, GPIO_PIN_RESET);
}
void Set_Valves1(uint32_t *pins, uint8_t pin_count) {
	for (uint8_t i = 0; i < pin_count; i++) {
		HAL_GPIO_WritePin(GPIOD, pins[i], GPIO_PIN_SET);
	}
}
void cycle1(uint32_t *pins, uint8_t pin_count) {
	Set_Valves(pins, pin_count);
	//osDelay(delay);
	//Reset_GPIO_Pins();
}
void cycle_for_rinse1(uint32_t *pins, uint8_t pin_count,uint32_t delay) {
	Set_Valves(pins, pin_count);
	osDelay(delay);
	Reset_GPIO_Pins();
}

//    -------------  This code for pressure sensor code  without while for using it weight measuring function ------------------

float Water_pressure_get_PSI(void)
{
    const uint32_t num_samples = 10;
    const float Vcc = 3.3f;   // MCU ADC reference voltage
    const float Vmin = 0.5f;  // Sensor output at 0 MPa
    const float Vmax = 4.5f;  // Sensor output at 1.6 MPa
    const float Pmax = 1.6f;  // Max pressure in MPa

    uint32_t raw = 0;

    // Take multiple samples and average
    for (uint32_t i = 0; i < num_samples; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
            raw += HAL_ADC_GetValue(&hadc1);
        }
    }
    raw /= num_samples;

    // Convert raw ADC to voltage
    float voltage = raw * (Vcc / 4095.0f);

    // Convert voltage to pressure (MPa)
    float pressMPa = (voltage - Vmin) * (Pmax / (Vmax - Vmin));

    // Clamp pressure
    if (pressMPa < 0.0f) pressMPa = 0.0f;
    if (pressMPa > Pmax) pressMPa = Pmax;

    // Convert MPa to PSI
    float pressPSI = pressMPa * 145.0377f;

    return pressPSI;
}
uint32_t Water_pressure_get_mmHg(void)
{
    const uint32_t num_samples = 10;
    const float Vcc = 3.3f;   // MCU ADC reference voltage
    const float Vmin = 0.5f;  // Sensor output at 0 MPa
    const float Vmax = 4.5f;  // Sensor output at 1.6 MPa
    const float Pmax = 1.6f;  // Max pressure in MPa

    uint32_t raw = 0;

    // Take multiple samples and average
    for (uint32_t i = 0; i < num_samples; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
            raw += HAL_ADC_GetValue(&hadc1);
        }
    }
    raw /= num_samples;

    // Convert raw ADC to voltage
    float voltage = raw * (Vcc / 4095.0f);

    // Convert voltage to pressure (MPa)
    float pressMPa = (voltage - Vmin) * (Pmax / (Vmax - Vmin));

    // Clamp pressure
    if (pressMPa < 0.0f) pressMPa = 0.0f;
    if (pressMPa > Pmax) pressMPa = Pmax;

    // Convert MPa to mmHg
    float press_mmHg = pressMPa * 7500.61683f;

    return (uint32_t)press_mmHg;
}

void WeightMeasurementTask(void) {
	int32_t offset = 0;
	float scale = NAN;
	float last_weight = 0.0;
	float weight = 0;
	float density = 1.0;
	float volume_ml;
	bool chemical_fill_needed=0;
    bool valve_active = 0;  // State flag for valve operation
    bool cycle6_active = 0; // Track if cycle 6 is active
    bool cycle2_done = 0;   // Flag to track if Cycle 2 (drain water) is complete
    bool cycle7_done = 0;   // Flag to track if Cycle 7 (fill water) is complete
    bool cycle8_done = 0;   // Flag to track if Cycle 8 (flush chemical A to V) is complete
    bool cycle9_done = 0;   // Flag to track if Cycle 9 (flush chemical V to A) is complete
    bool cycle10_done = 0;  // Flag to track if Cycle 10 (flush chemical DO to DI) is complete
    bool pressure_checked = 0; // Flag to track if the pressure condition has been handled

	FLASH_FlushCaches();
	// Read saved offset and scale from Flash
	Flash_ReadData(FLASH_USER_START_ADDR, &offset, sizeof(offset));
	Flash_ReadData(FLASH_USER_START_ADDR + sizeof(offset), &scale,
			sizeof(scale));

	if (offset == -1 || isnan(scale)) {

		// Perform first-time calibration
		// printf("First-time calibration needed.\n");
//		snprintf(buffer0, sizeof(buffer0),
//				"First-time calibration needed.\n\r:");
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer0, strlen(buffer0),
//		HAL_MAX_DELAY);

		//printf("Calibrating offset...\n");
//		snprintf(buffer1, sizeof(buffer1), "Calibrating offset...\n\r:");
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer1, strlen(buffer1),
//		HAL_MAX_DELAY);

		osDelay(50);
		offset = HX711_Read();
		/*
		 //printf("Offset: %ld\n", offset);
		 snprintf(buffer2, sizeof(buffer2), "Offset: %ld\n\r:");
		 HAL_UART_Transmit(&huart1, (uint8_t*) buffer2, strlen(buffer2),
		 HAL_MAX_DELAY);

		 //printf("Place known weight (e.g., 2kg) on the scale...\n");
		 snprintf(buffer3, sizeof(buffer3),
		 "Place known weight (e.g., 2kg) on the scale...\n\r:");
		 HAL_UART_Transmit(&huart1, (uint8_t*) buffer3, strlen(buffer3),
		 HAL_MAX_DELAY);*/

//		int32_t raw_with_weight = HX711_Read();
//		scale = (float) (raw_with_weight - offset) / 346.0; // Known weight: 2kg
//		// printf("Scale factor: %.2f\n", scale);
//		snprintf(buffer4, sizeof(buffer4), "Scale factor: %.2f\n\r:");
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer4, strlen(buffer4),
//		HAL_MAX_DELAY);
		// Save offset and scale to Flash
		Flash_WriteData(FLASH_USER_START_ADDR, &offset, sizeof(offset));
		Flash_WriteData(FLASH_USER_START_ADDR + sizeof(offset), &scale,
				sizeof(scale));

	} else {
		//printf("Offset and scale loaded from Flash.\n");
//		snprintf(buffer5, sizeof(buffer5),
//				"Offset and scale loaded from Flash.\n\r:");
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5),
//		HAL_MAX_DELAY);
		//// printf("Offset: %ld, Scale: %.2f\n", offset, scale);

		snprintf(buffer6, sizeof(buffer6), "Offset: %ld\n\r:", offset);
		HAL_UART_Transmit(&huart1, (uint8_t*) buffer6, strlen(buffer6),
		HAL_MAX_DELAY);
		snprintf(buffer7, sizeof(buffer7), "Scale: %.2f\n\r:", scale);
		HAL_UART_Transmit(&huart1, (uint8_t*) buffer7, strlen(buffer7),
		HAL_MAX_DELAY);

	}


    // Valve control pattern cycles
    //******************  Rinse & Pressure check cycle  *********************//
	uint32_t cycle_1[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_9,GPIO_PIN_11}; // STEP 1 FLUSH THE WATER A to V FOR 30 SECOND
    uint32_t cycle_2[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_4,GPIO_PIN_8,GPIO_PIN_11}; // STEP 2 FLUSH THE WATER V to A FOR 30 SECOND
    uint32_t cycle_3[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_3,GPIO_PIN_12,GPIO_PIN_11}; // SETP 3 FLUSH THE WATER FOR 30 DO to DI
    uint32_t cycle_4[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_13,GPIO_PIN_11}; // STEP 4: Fill the water
    uint32_t cycle_5[] = {GPIO_PIN_13,GPIO_PIN_4,GPIO_PIN_9}; // STEP 5: Drain the water
    uint32_t cycle_6[] = {GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_13}; // STEP 6: Fill chemical
    uint32_t cycle_7[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_13,GPIO_PIN_11}; // STEP 7: Fill the water again
    uint32_t cycle_8[] = {GPIO_PIN_13,GPIO_PIN_2,GPIO_PIN_9}; // STEP 8: 200ml chemical water flush A to V
    uint32_t cycle_9[] = {GPIO_PIN_13,GPIO_PIN_4,GPIO_PIN_8}; // STEP 9: 200ml chemical water flush V to A
    uint32_t cycle_10[] = {GPIO_PIN_13,GPIO_PIN_3,GPIO_PIN_12}; // STEP 10: 250ml chemical water flush DO to DI
      // Delay between cycles
   // Draincycle();
    SendIconToDWIN();

    // State variables (add these at the top with your other declarations)
    bool initial_fill_done = false;
    bool drain_complete = false;
    bool chemical_fill_done = false;
    bool second_fill_done = false;
    bool flush_a_to_v_done = false;
    bool flush_v_to_a_done = false;
    bool flush_do_to_di_done = false;


	while (1) {
		/* USER CODE END WHILE */


		weight = HX711_GetWeight(offset, scale);
		get_wieght = weight;


//		if (weight > 3000.0) {
//			snprintf(buffer, sizeof(buffer),"Overload! Max weight is 2kg.\n\r");
//			HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer),HAL_MAX_DELAY);}

		 pressPSI = Water_pressure_get_mmHg();  // Get latest PSI

		 snprintf(buf, sizeof(buf), "Water Pressure: %f mmHg\r\n", pressPSI);
		 HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
		 snprintf(buffer, sizeof(buffer),"                                                   Weight: %.f ml\n\r",get_wieght);
		 HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer),HAL_MAX_DELAY);

//  -------- Here is code cycle will be run according to weight which we are measure from the load cell ---------
		/* if (get_wieght >= 1500) {
		     snprintf(buffer5,sizeof(buffer5),"Drain condition detected\n\r");
		     HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5), HAL_MAX_DELAY);

		     // Reset pump and stop filling
		     Reset_GPIO_Pins1();
		     cycle1(cycle_5, sizeof(cycle_5) / sizeof(cycle_5[0]));
		     cycle2_done = 1;  // Mark Cycle 2 as done
		     chemical_fill_needed = 1;  // Flag that chemical fill is needed after drain

		     // Skip other checks until weight drops
		     osDelay(250);
		     continue;
		 }

		 // After drain completes and weight returns to initial level
		 if (get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
		     if (!cycle2_done) {
		         // Start initial water fill cycle
		         cycle1(cycle_4, sizeof(cycle_4) / sizeof(cycle_4[0]));
		         snprintf(buffer5, sizeof(buffer5), "Starting fill cycle: %.f\n\r", get_wieght);
		         HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5), HAL_MAX_DELAY);
		     }
		     else if (chemical_fill_needed) {
		         // Chemical fill phase after drain completes
		         Reset_GPIO_Pins1();
		         cycle6_active = 1;
		         chemical_fill_needed = 0;  // Reset the flag
		         cycle1(cycle_6, sizeof(cycle_6) / sizeof(cycle_6[0]));
		         snprintf(buffer5, sizeof(buffer5), "Starting chemical fill: %.f\n\r", get_wieght);
		         HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5), HAL_MAX_DELAY);
		     }
		 }

		 // Chemical fill completion check
		 if (cycle6_active && get_wieght >= (1053 - TOLERANCE) && get_wieght <= (1053 + TOLERANCE)) {
		     cycle6_active = 0;
		     cycle7_done = 1;
		     Reset_GPIO_Pins1();
		     cycle1(cycle_7, sizeof(cycle_7) / sizeof(cycle_7[0]));
		     snprintf(buffer5, sizeof(buffer5), "Chemical fill complete, starting water fill\n\r");
		     HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5), HAL_MAX_DELAY);
		 }
		 // After chemical filling is complete (≈1053ml) and water filling starts
		 if (cycle7_done && get_wieght >= (1644 - TOLERANCE) && get_wieght <= (1644 + TOLERANCE) && !cycle8_done) {
		     // Stop water filling and prepare for chemical flush
		     Reset_GPIO_Pins1();
		     snprintf(buffer, sizeof(buffer), "Water fill complete (%.1f ml), starting chemical flush A→V\n\r", get_wieght);
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

		     // Start chemical flush A to V
		     cycle1(cycle_8, sizeof(cycle_8) / sizeof(cycle_8[0]));
		     cycle8_done = 1;

		     snprintf(buffer, sizeof(buffer), "Chemical flush A→V started (target: 200ml reduction)\n\r");
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		 }

		 // Chemical flush A→V completion check (target ≈1444ml)
		 if (cycle8_done && get_wieght >= (1444 - TOLERANCE) && get_wieght <= (1444 + TOLERANCE) && !cycle9_done) {
		     Reset_GPIO_Pins1();

		     snprintf(buffer, sizeof(buffer), "Chemical A→V flush complete (%.1f ml), starting V→A flush\n\r", get_wieght);
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

		     // Start chemical flush V to A
		     cycle1(cycle_9, sizeof(cycle_9) / sizeof(cycle_9[0]));
		     cycle9_done = 1;

		     snprintf(buffer, sizeof(buffer), "Chemical flush V→A started (target: 200ml reduction)\n\r");
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		 }

		 // Chemical flush V→A completion check (target ≈1244ml)
		 if (cycle9_done && get_wieght >= (1244 - TOLERANCE) && get_wieght <= (1244 + TOLERANCE) && !cycle10_done) {
		     Reset_GPIO_Pins1();

		     snprintf(buffer, sizeof(buffer), "Chemical V→A flush complete (%.1f ml), starting DO→DI flush\n\r", get_wieght);
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

		     // Start final chemical flush DO to DI
		     cycle1(cycle_10, sizeof(cycle_10) / sizeof(cycle_10[0]));
		     cycle10_done = 1;

		     snprintf(buffer, sizeof(buffer), "Chemical flush DO→DI started (target: 250ml reduction)\n\r");
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		 }

		 // Final completion check (back to ≈1035ml)
		 if (cycle10_done && get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
		     Reset_GPIO_Pins1();

		     snprintf(buffer, sizeof(buffer), "Process complete! Final weight: %.1f ml\n\r", get_wieght);
		     HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

		     // Reset all flags for next cycle
		     cycle2_done = 0;
		     cycle6_active = 0;
		     cycle7_done = 0;
		     cycle8_done = 0;
		     cycle9_done = 0;
		     cycle10_done = 0;
		     chemical_fill_needed = 0;
		 }*/
		   // Step 1: Initial water filling (1035ml → 1500ml)
		 // Step 1: Initial water filling (1035ml → 1500ml)
		     if (!initial_fill_done && get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_4, sizeof(cycle_4) / sizeof(cycle_4[0]));
		         snprintf(buffer, sizeof(buffer), "Starting initial water filling\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         initial_fill_done = true;
		     }

		     // Step 2: Drain when reaching 1500ml
		     if (initial_fill_done && !drain_complete && get_wieght >= 1500) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_5, sizeof(cycle_5) / sizeof(cycle_5[0]));
		         snprintf(buffer, sizeof(buffer), "Draining water (reached %.1f ml)\n\r", get_wieght);
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         drain_complete = true;
		     }

		     // Step 3: After drain completes (back to ~1035ml), start chemical fill
		     if (drain_complete && !chemical_fill_done &&
		         get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_6, sizeof(cycle_6) / sizeof(cycle_6[0]));
		         snprintf(buffer, sizeof(buffer), "Starting chemical filling\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         chemical_fill_done = true;
		     }

		     // Step 4: When chemical fill reaches 1053ml, start second water fill
		     if (chemical_fill_done && !second_fill_done &&
		         get_wieght >= (1053 - TOLERANCE) && get_wieght <= (1053 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_7, sizeof(cycle_7) / sizeof(cycle_7[0]));
		         snprintf(buffer, sizeof(buffer), "Chemical fill complete, starting water filling\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         second_fill_done = true;
		     }

		     // Step 5: When second fill reaches 1644ml, start A→V flush
		     if (second_fill_done && !flush_a_to_v_done &&
		         get_wieght >= (1651 - TOLERANCE) && get_wieght <= (1651 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_8, sizeof(cycle_8) / sizeof(cycle_8[0]));
		         snprintf(buffer, sizeof(buffer), "Water fill complete, flushing A→V (200ml)\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         flush_a_to_v_done = true;
		     }

		     // Step 6: When A→V flush reaches 1444ml, start V→A flush
		     if (flush_a_to_v_done && !flush_v_to_a_done &&
		         get_wieght >= (1444 - TOLERANCE) && get_wieght <= (1444 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_9, sizeof(cycle_9) / sizeof(cycle_9[0]));
		         snprintf(buffer, sizeof(buffer), "A→V flush complete, flushing V→A (200ml)\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         flush_v_to_a_done = true;
		     }

		     // Step 7: When V→A flush reaches 1244ml, start DO→DI flush
		     if (flush_v_to_a_done && !flush_do_to_di_done &&
		         get_wieght >= (1244 - TOLERANCE) && get_wieght <= (1244 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         cycle1(cycle_10, sizeof(cycle_10) / sizeof(cycle_10[0]));
		         snprintf(buffer, sizeof(buffer), "V→A flush complete, flushing DO→DI (250ml)\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		         flush_do_to_di_done = true;
		     }

		     // Step 8: When DO→DI flush returns to 1035ml, reset all flags to restart cycle
		     if (flush_do_to_di_done &&
		         get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
		         Reset_GPIO_Pins1();
		         snprintf(buffer, sizeof(buffer), "Process complete! Ready for next cycle\n\r");
		         HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

		         // Reset all flags to restart the cycle
		         initial_fill_done = false;
		         drain_complete = false;
		         chemical_fill_done = false;
		         second_fill_done = false;
		         flush_a_to_v_done = false;
		         flush_v_to_a_done = false;
		         flush_do_to_di_done = false;
		     }

		// if (get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {

//			 cycle_for_rinse1(cycle_1, sizeof(cycle_1) / sizeof(cycle_1[0]), 20000);
//	         osDelay(5000); // Hold for 30 seconds
//			 cycle_for_rinse1(cycle_2, sizeof(cycle_2) / sizeof(cycle_2[0]), 20000);
//	         osDelay(5000); // Hold for 30 seconds
//
//			 cycle_for_rinse1(cycle_3, sizeof(cycle_3) / sizeof(cycle_3[0]), 20000);
//	         osDelay(5000); // Hold for 30 seconds

/*
	        if (!cycle2_done && get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
	            // Start filling water cycle (Cycle 1) when weight is close to
	            cycle1(cycle_4, sizeof(cycle_4) / sizeof(cycle_4[0]));
	            snprintf(buffer5, sizeof(buffer5), "Weight exceeded threshold: %.f\n\r", get_wieght);
	           	HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5), HAL_MAX_DELAY);

	        }
	        if (get_wieght >= 1500) {

	      	    	        	snprintf(buffer5,sizeof(buffer5),"detected\n\r");
	      	    	        	HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5), HAL_MAX_DELAY);
	      	    	            // Reset pump and stop filling
	      	    	            Reset_GPIO_Pins1();
	      	    	           // osDelay(30000); // Hold for 30 seconds

	      	    	            // Proceed with cycle
	      	    	            cycle1(cycle_5, sizeof(cycle_5) / sizeof(cycle_5[0]));
	      	    	           // osDelay(30000); // Hold for 1 minute
	      	    	            cycle2_done = 1;  // Mark Cycle 2 as done
	      	    	        }
*/


				/*static uint32_t cycle2_start_time = 0;

				if (cycle2_start_time == 0) {
					// First entry into this block - record start time and reset GPIO
					Reset_GPIO_Pins1();

					cycle2_start_time = HAL_GetTick();
				} else {
					// Check if 30 seconds have passed
					if (HAL_GetTick() - cycle2_start_time >= 30000) {
						// 30 seconds have elapsed - proceed with drain cycle
						Draincycle();
						cycle2_done = 1; // Mark Cycle 2 as complete
						cycle2_start_time = 0; // Reset for potential future use
					}
				}
			}*/

	      /*
	        if (cycle2_done && !cycle6_active && get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
	            // Start cycle 6 (fill 15ml chemical) when weight is near
	            Reset_GPIO_Pins1();
	            cycle6_active = 1;
	            cycle1(cycle_6, sizeof(cycle_6) / sizeof(cycle_6[0]));
	        }

	        if (cycle6_active && get_wieght >= (1053 - TOLERANCE) && get_wieght <= (1053 + TOLERANCE)) {
	            // Stop chemical filling and start cycle 7 (fill 635ml water)
	            cycle6_active = 0;
	            cycle7_done = 1; // Mark Cycle 6 as done
	            Reset_GPIO_Pins1();
	            cycle1(cycle_7, sizeof(cycle_7) / sizeof(cycle_7[0]));
	        }

	        if (cycle7_done && get_wieght >= (1644 - TOLERANCE) && get_wieght <= (1644 + TOLERANCE) && !cycle8_done) {
	            // Stop filling water and hold for 30 seconds (Cycle 7)
	            Reset_GPIO_Pins1();
	            //osDelay(20000); // Hold for 30 seconds
	            cycle1(cycle_8, sizeof(cycle_8) / sizeof(cycle_8[0])); // Start cycle 8 (flush chemical A to V)
	            cycle8_done = 1; // Mark Cycle 8 as done
	        }

	        if (cycle8_done && get_wieght >= (1442 - TOLERANCE) && get_wieght <= (1442 + TOLERANCE) && !cycle9_done) {
	            // Start cycle 9 (flush chemical V to A)
	            Reset_GPIO_Pins1();
	        	//osDelay(10000);
	            cycle1(cycle_9, sizeof(cycle_9) / sizeof(cycle_9[0])); // Flush 200ml chemical water (V to A)
	            cycle9_done = 1; // Mark Cycle 9 as done
	        }

	        if (cycle9_done && get_wieght >= (1240 - TOLERANCE) && get_wieght <= (1240 + TOLERANCE) && !cycle10_done) {
	            // Start cycle 10 (flush 250ml chemical water DO to DI)
	            Reset_GPIO_Pins1();
	        	//osDelay(10000);
	            cycle1(cycle_10, sizeof(cycle_10) / sizeof(cycle_10[0])); // Flush 250ml chemical water (DO to DI)
	            cycle10_done = 1; // Mark Cycle 10 as done
	        }

	        // --- Complete Cycle ---
	        if (cycle10_done && get_wieght >= (1035 - TOLERANCE) && get_wieght <= (1035 + TOLERANCE)) {
	            // All cycles complete, reset valves to their initial state
	            Reset_GPIO_Pins1();
	            cycle2_done = 0;
	            cycle6_active = 0;
	            cycle7_done = 0;
	            cycle8_done = 0;
	            cycle9_done = 0;
	            cycle10_done = 0;
	        }
		   }*/


		 osDelay(250);

	}
}
//  ------------ This is the pressure sensor code for taking the value in while loop ------------
int Water_pressure_data(void)
{
	HAL_ADC_Start(&hadc1);
	uint32_t num_samples = 10;
	const float Vcc = 3.3f;   // ADC reference = sensor V+ = 5 V
	const float Vmin = 0.5f;   // 0 MPa → 0.5 V
	const float Vmax = 4.5f;   // 1.6 MPa → 4.5 V
	const float Pmax = 1.6f;   // full-scale pressure [MPa]

	char buf[100];
	uint32_t raw = 0;
	float voltage, pressMPa, presskPa, pressPSI;

	for (;;) {
		for (int i = 0; i < num_samples; i++) {
			HAL_ADC_Start(&hadc1);
			if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
				raw = HAL_ADC_GetValue(&hadc1);
			}

			//HAL_ADC_Stop(&hadc1);
		}
		// Convert raw ADC to voltage
		 voltage = raw * (Vcc / 4095.0f);

			 	  // 3) Convert voltage to pressure (MPa)
			 	  pressMPa = (voltage - Vmin) * (Pmax / (Vmax - Vmin));

			 	  // Clamp pressure to valid range
			 	  if (pressMPa < 0.0f) pressMPa = 0.0f;
			 	  if (pressMPa > Pmax) pressMPa = Pmax;

			 	  // 4) Convert MPa to other units
			 	  presskPa = pressMPa * 1000.0f;       // 1 MPa = 1000 kPa
			 	  pressPSI = pressMPa * 145.0377f;      // 1 MPa ≈ 145.0377 PSI

			 	  // (Optional) PSI range validation
			 	  const float minPSI = 0.0f;            // Minimum allowed PSI
			 	  const float maxPSI = 250.0f;
			       /* 5) Print out via UART */
			       int len = snprintf(buf, sizeof(buf),
			       "RAW=%4lu  V=%.3f V  prssure %.1f PSI \r\n",
			       raw, voltage,pressPSI);
			       HAL_UART_Transmit(&huart1, (uint8_t*) buf, len, HAL_MAX_DELAY);

			       osDelay(100);

	}
}


// ---------This code for rinse and cleaning cycle with common function which will be use blocking delay function--------------
/*
void Reset_GPIO_Pins(void) {
	HAL_GPIO_WritePin(GPIOD,
			GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
					| GPIO_PIN_5| GPIO_PIN_8 | GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,
			GPIO_PIN_RESET);
}
void Set_Valves(uint32_t *pins, uint8_t pin_count) {
	for (uint8_t i = 0; i < pin_count; i++) {
		HAL_GPIO_WritePin(GPIOD, pins[i], GPIO_PIN_SET);
	}
}
void cycle(uint32_t *pins, uint8_t pin_count,uint32_t delay) {
	Set_Valves(pins, pin_count);
	osDelay(delay);
	Reset_GPIO_Pins();
}
int ValveOperatingTask(void) {
	//uint32_t cycle_1[] = {GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_2,GPIO_PIN_5,GPIO_PIN_10}; //Tank input line connection s_
//	uint32_t cycle_2[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_13,GPIO_PIN_11}; // STEP 1 FLUSH THE WATER A to V FOR 30 SECOND
//	uint32_t cycle_3[] = {GPIO_PIN_13,GPIO_PIN_4}; //STEP 9  200ml CHEMICAL WATER FLUSH V to A
//	uint32_t cycle_4[] ={GPIO_PIN_13,GPIO_PIN_3,GPIO_PIN_12}; //STEP 10 250ml CHEMICAL WATER FLUSH DO to DI
	    //******************  Rinse & Pressure check cycle  *********************/
	//	uint32_t cycle_1[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_9,GPIO_PIN_11}; // STEP 1 FLUSH THE WATER A to V FOR 30 SECOND
//		uint32_t cycle_2[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_4,GPIO_PIN_8,GPIO_PIN_11}; // STEP 2 FLUSH THE WATER V to A FOR 30 SECOND
//	    uint32_t cycle_3[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_3,GPIO_PIN_12,GPIO_PIN_11}; // SETP 3 FLUSH THE WATER FOR 30 DO to DI
	//******************  Cleaning cycle start from here  ********************//
	//	uint32_t cycle_4[] = {GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_2,GPIO_PIN_5,GPIO_PIN_10}; //Tank input line connection s_
//	uint32_t cycle_4[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_5,GPIO_PIN_10, GPIO_PIN_11 };	// STEP4 FILL THE 1000ml WATER IN THE TANK AND HOLD IT FOR I MINUTES
//	uint32_t cycle_5[] = { GPIO_PIN_13,GPIO_PIN_2, GPIO_PIN_9}; //STEP 5 DRIAN OUT THE WATER FROM THE TANK TO CHECK TANK VALUME
//	uint32_t cycle_6[] = { GPIO_PIN_15, GPIO_PIN_14, GPIO_PIN_2, GPIO_PIN_5,GPIO_PIN_10 }; // STEP 6 FILL THE 15ml CHEMICAL IN THE TANK
//	uint32_t cycle_7[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_5,GPIO_PIN_10, GPIO_PIN_11 }; // STEP 7 FILL THE 635ml WATER IN THR TANK
//
//	uint32_t cycle_8[] = { GPIO_PIN_13, GPIO_PIN_2, GPIO_PIN_9 }; // STEP 8 200ml CHEMICAL WATER FLUSH A to V
//	uint32_t cycle_9[] = { GPIO_PIN_13, GPIO_PIN_4, GPIO_PIN_8 }; //STEP 9  200ml CHEMICAL WATER FLUSH V to A
//	uint32_t cycle_10[] = { GPIO_PIN_13, GPIO_PIN_3, GPIO_PIN_12 }; //STEP 10 250ml CHEMICAL WATER FLUSH DO to DI
//
	//******************  TCV cycle start from here  ********************//

//	    uint32_t cycle_11[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_11,GPIO_PIN_9}; //STEP 11 FLUSH THE WATER A to V FOR THE 10 SECOND
//	    uint32_t cycle_12[] = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_4,GPIO_PIN_8,GPIO_PIN_11};
//	    uint32_t cycle_13[] = {GPIO_PIN_4,GPIO_PIN_2,GPIO_PIN_5,GPIO_PIN_10};

	//******************  Pressure leack test cycle start from here  ********************//

//	    uint32_t cycle_13[] ={GPIO_PIN_4,GPIO_PIN_3, GPIO_PIN_12}; //STEP 13 -250mmHG pressure create
//	    uint32_t cycle_14[] = {GPIO_PIN_4};

	//******************  Sterilization cycle start from here  ********************//

//			GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13,ut GPIO_PIN_14 };
//	uint32_t cycle_4[] = { GPIO_PIN_0, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10,
//			GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13 };
//	uint32_t cycle_5[] = { GPIO_PIN_0, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10,
//			GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13 };
//	uint32_t cycle_6[] = { GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_11, GPIO_PIN_12,
//			GPIO_PIN_14, GPIO_PIN_15 };
//	uint32_t cycle_7[] = { GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_11, GPIO_PIN_12,
//			GPIO_PIN_14, GPIO_PIN_15 };

	/*while (1) {
		//cycle(cycle_1, sizeof(cycle_1) / sizeof(cycle_1[0]), 2000);
		//vTaskDelay(pdMS_TO_TICKS(10000));
//		cycle(cycle_2, sizeof(cycle_2) / sizeof(cycle_2[0]), 10000);
//		vTaskDelay(pdMS_TO_TICKS(10000));
//		//cycle(cycle_3, sizeof(cycle_3) / sizeof(cycle_3[0]), 10000);
//		//vTaskDelay(pdMS_TO_TICKS(10000));
//		cycle(cycle_1, sizeof(cycle_1) / sizeof(cycle_1[0]), 30000);
//		vTaskDelay(pdMS_TO_TICKS(5000));
//		cycle(cycle_2, sizeof(cycle_2) / sizeof(cycle_2[0]), 7000);
//		vTaskDelay(pdMS_TO_TICKS(3000));
//		cycle(cycle_3, sizeof(cycle_3) / sizeof(cycle_3[0]), 30000);
//		vTaskDelay(pdMS_TO_TICKS(30000));
//		cycle(cycle_4, sizeof(cycle_4) / sizeof(cycle_4[0]), 10000);
//		vTaskDelay(pdMS_TO_TICKS(60000));
		cycle(cycle_5, sizeof(cycle_5) / sizeof(cycle_5[0]), 30000);
		vTaskDelay(pdMS_TO_TICKS(20000));
//		cycle(cycle_6, sizeof(cycle_6) / sizeof(cycle_6[0]), 10000);
//		vTaskDelay(pdMS_TO_TICKS(20000));
//		cycle(cycle_7, sizeof(cycle_7) / sizeof(cycle_7[0]), 22000);
//    	vTaskDelay(pdMS_TO_TICKS(20000));
//		cycle(cycle_8, sizeof(cycle_8) / sizeof(cycle_8[0]), 10000);
//		vTaskDelay(pdMS_TO_TICKS(10000));
//		cycle(cycle_9, sizeof(cycle_9) / sizeof(cycle_9[0]), 10000);
//	    vTaskDelay(pdMS_TO_TICKS(10000));
//	    cycle(cycle_10, sizeof(cycle_10) / sizeof(cycle_10[0]),38000);
//		vTaskDelay(pdMS_TO_TICKS(10000));
//		cycle(cycle_11, sizeof(cycle_11) / sizeof(cycle_11[0]),20000);
//      vTaskDelay(pdMS_TO_TICKS(10000));
//      cycle(cycle_12, sizeof(cycle_12) / sizeof(cycle_12[0]),10000);
//      vTaskDelay(pdMS_TO_TICKS(10000));
//      cycle(cycle_13, sizeof(cycle_13) / sizeof(cycle_13[0]),60000);
//      vTaskDelay(pdMS_TO_TICKS(10000));
	}
}
*/
//  -------------------  These are the common function which will be use for rinse cycle and cleaning cycle  ------------------------

void Set_Valves(uint32_t *pins, uint8_t pin_count) {
    for (uint8_t i = 0; i < pin_count; i++) {
        HAL_GPIO_WritePin(GPIOD, pins[i], GPIO_PIN_SET);  // Set pins high
    }
}

void Reset_GPIO_Pins(uint32_t *pins, uint8_t pin_count) {
    for (uint8_t i = 0; i < pin_count; i++) {
        HAL_GPIO_WritePin(GPIOD, pins[i], GPIO_PIN_RESET);
    }
}

void cycle(uint32_t *pins, uint8_t pin_count, uint32_t delay, uint32_t *last_time, uint32_t *end_time) {
    uint32_t current_time = HAL_GetTick();

    if (*last_time == 0) {
        Set_Valves(pins, pin_count);
        *last_time = current_time;
        *end_time = current_time + delay;
    }

    if (current_time >= *end_time) {
        Reset_GPIO_Pins(pins, pin_count);  // Only reset pins belonging to this cycle
        *last_time = 0;
    }
}

// -------------------- And here is the nonblocking code for risen & cleaning cycle code ----------------
/*
void ValveOperatingTask(void) {
    // Define all valve cycles with their durations (in milliseconds)
    static const struct {
        uint32_t pins[6];       // Pin configuration
        uint8_t pin_count;      // Number of pins
        uint32_t duration_ms;   // Custom duration for each cycle
    } cycles[] = {
        // Flushing cycles (30 seconds each)
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_9, GPIO_PIN_11, 0}, 5, 20000}, // Cycle 1
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_8, GPIO_PIN_11, 0}, 5, 20000},  // Cycle 2
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_3, GPIO_PIN_12, GPIO_PIN_11, 0}, 5,20000}, // Cycle 3

        // Cleaning cycles
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_5, GPIO_PIN_10, GPIO_PIN_11}, 6, 15000}, // Cycle 4 (30s)
        {{GPIO_PIN_13, GPIO_PIN_2, GPIO_PIN_9, 0, 0, 0}, 3, 20000},                              // Cycle 5 (5s)
        {{GPIO_PIN_15, GPIO_PIN_14, GPIO_PIN_2, GPIO_PIN_5, GPIO_PIN_10, 0}, 5, 5000},         // Cycle 6 (30s)
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_5, GPIO_PIN_10, GPIO_PIN_11}, 6, 20000}, // Cycle 7 (30s)

        // Chemical flush cycles
        {{GPIO_PIN_13, GPIO_PIN_2, GPIO_PIN_9, 0, 0, 0}, 3, 10000},                             // Cycle 8 (10s)
        {{GPIO_PIN_13, GPIO_PIN_4, GPIO_PIN_8, 0, 0, 0}, 3, 10000},                             // Cycle 9 (10s)
        {{GPIO_PIN_13, GPIO_PIN_3, GPIO_PIN_12, 0, 0, 0}, 3, 20000},                           // Cycle 10 (20s)

        // TCV cycles (30 seconds each)
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_11, GPIO_PIN_9, 0}, 5, 30000},          // Cycle 11
        {{GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_8, GPIO_PIN_11, 0}, 5, 30000}           // Cycle 12
    };

    enum {
        STATE_INITIAL,
        STATE_RUN_CYCLE,
        STATE_REGULAR_DELAY,
        STATE_CYCLE4_DELAY  // Special 1-minute delay after Cycle 4
    } state = STATE_INITIAL;

    static uint8_t current_cycle = 0;
    static uint32_t cycle_end_time = 0;
    static uint32_t delay_end_time = 0;

    const uint32_t regular_delay = 500;    // 0.5 seconds between most cycles
    const uint32_t cycle4_delay = 60000;   // 1 minute delay after Cycle 4

    // Initialize all valves to OFF state
    for (uint8_t i = 0; i < sizeof(cycles)/sizeof(cycles[0]); i++) {
        Reset_GPIO_Pins(cycles[i].pins, cycles[i].pin_count);
    }

    while (1) {
        uint32_t current_time = HAL_GetTick();

        switch (state) {
            case STATE_INITIAL:
                state = STATE_RUN_CYCLE;
                break;

            case STATE_RUN_CYCLE:
                if (cycle_end_time == 0) {
                    // Start current cycle with its custom duration
                    Set_Valves(cycles[current_cycle].pins, cycles[current_cycle].pin_count);
                    cycle_end_time = current_time + cycles[current_cycle].duration_ms;
                }
                else if (current_time >= cycle_end_time) {
                    // End current cycle
                    Reset_GPIO_Pins(cycles[current_cycle].pins, cycles[current_cycle].pin_count);

                    // Special handling after Cycle 4
                    if (current_cycle == 3) {  // Cycle 4 is index 3
                        delay_end_time = current_time + cycle4_delay;
                        state = STATE_CYCLE4_DELAY;
                    }
                    else {
                        // Regular delay for other cycles
                        delay_end_time = current_time + regular_delay;
                        state = STATE_REGULAR_DELAY;
                    }
                    cycle_end_time = 0;
                }
                break;

            case STATE_REGULAR_DELAY:
            case STATE_CYCLE4_DELAY:
                if (current_time >= delay_end_time) {
                    // Move to next cycle (with wrap-around)
                    current_cycle = (current_cycle + 1) % (sizeof(cycles)/sizeof(cycles[0]));
                    state = STATE_RUN_CYCLE;
                }
                break;
        }
    }
}*/
         // -------------- This code nonblocking code for rinse cycle ----------------------

void ValveOperatingTask(void) {
    // Define all needed cycles
    static const uint32_t cycle2_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_3, GPIO_PIN_12,GPIO_PIN_11};
    static const uint32_t cycle3_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_13,GPIO_PIN_11};
    static const uint32_t cycle4_pins[] = {GPIO_PIN_13, GPIO_PIN_4, GPIO_PIN_9};

    // Valve cycles with their durations
    static const struct {
        const uint32_t* pins;
        uint8_t pin_count;
        uint32_t duration_ms;
    } cycles[] = {
        { cycle2_pins, sizeof(cycle2_pins)/sizeof(cycle2_pins[0]), 5000 },    // Cycle 2: 5 sec
        { cycle3_pins, sizeof(cycle3_pins)/sizeof(cycle3_pins[0]), 10000 },   // Cycle 3: 10 sec
        { cycle4_pins, sizeof(cycle4_pins)/sizeof(cycle4_pins[0]), 100000 }   // Cycle 4: 100 sec
    };

    enum {
        STATE_INITIAL,
        STATE_RUN_CYCLE,
        STATE_INTER_CYCLE_DELAY,  // 5s delay between cycles
        STATE_POST_CYCLE3_DELAY,  // 1 minute delay after cycle 3
        STATE_POST_CYCLE4_DELAY   // 30 second delay after cycle 4
    } state = STATE_INITIAL;

    static uint8_t current_cycle = 0;
    static uint32_t cycle_end_time = 0;
    static uint32_t delay_end_time = 0;

    const uint32_t switching_delay = 5000;      // 5 seconds between cycles
    const uint32_t post_cycle3_delay = 60000;  // 1 minute after cycle 3
    const uint32_t post_cycle4_delay = 30000;  // 30 seconds after cycle 4

    // Initialize all valves to OFF state
    for (uint8_t i = 0; i < sizeof(cycles)/sizeof(cycles[0]); i++) {
        Reset_GPIO_Pins(cycles[i].pins, cycles[i].pin_count);
    }

    while (1) {
        uint32_t current_time = HAL_GetTick();

        switch (state) {
            case STATE_INITIAL:
                state = STATE_RUN_CYCLE;
                break;

            case STATE_RUN_CYCLE:
                if (cycle_end_time == 0) {
                    // Start current cycle
                    Set_Valves(cycles[current_cycle].pins, cycles[current_cycle].pin_count);
                    cycle_end_time = current_time + cycles[current_cycle].duration_ms;
                }
                else if (current_time >= cycle_end_time) {
                    // Cycle completed
                    Reset_GPIO_Pins(cycles[current_cycle].pins, cycles[current_cycle].pin_count);

                    if (current_cycle == 1) {  // After cycle 3
                        delay_end_time = current_time + post_cycle3_delay;
                        state = STATE_POST_CYCLE3_DELAY;
                    }
                    else if (current_cycle == 2) {  // After cycle 4
                        delay_end_time = current_time + post_cycle4_delay;
                        state = STATE_POST_CYCLE4_DELAY;
                    }
                    else {  // After cycle 2
                        delay_end_time = current_time + switching_delay;
                        state = STATE_INTER_CYCLE_DELAY;
                    }
                    cycle_end_time = 0;
                }
                break;

            case STATE_INTER_CYCLE_DELAY:
            case STATE_POST_CYCLE3_DELAY:
            case STATE_POST_CYCLE4_DELAY:
                if (current_time >= delay_end_time) {
                    // Move to next cycle (with wrap-around)
                    current_cycle = (current_cycle + 1) % (sizeof(cycles)/sizeof(cycles[0]));
                    state = STATE_RUN_CYCLE;
                }
                break;
        }
    }
}
void Rinsecycle(void) {
    // Define all cycles
    static const uint32_t cycle1_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_9, GPIO_PIN_11};
    static const uint32_t cycle2_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_8, GPIO_PIN_11};
    static const uint32_t cycle3_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_3, GPIO_PIN_12, GPIO_PIN_11};

    // Valve cycles with their durations
    static const struct {
        const uint32_t* pins;
        uint8_t pin_count;
        uint32_t duration_ms;
    } cycles[] = {
        { cycle1_pins, sizeof(cycle1_pins)/sizeof(cycle1_pins[0]), 30000 },  // Cycle 1: 30 sec
        { cycle2_pins, sizeof(cycle2_pins)/sizeof(cycle2_pins[0]), 30000 },  // Cycle 2: 30 sec
        { cycle3_pins, sizeof(cycle3_pins)/sizeof(cycle3_pins[0]), 30000 }   // Cycle 3: 30 sec
    };

    static enum {
        STATE_INITIAL,
        STATE_RUN_CYCLE,
        STATE_INTER_CYCLE_DELAY,
        STATE_COMPLETE
    } state = STATE_INITIAL;

    static uint8_t current_cycle = 0;
    static uint32_t cycle_end_time = 0;
    static uint32_t delay_end_time = 0;
    const uint32_t switching_delay = 10000;  // 10 seconds between cycles

    uint32_t current_time = HAL_GetTick();

    switch (state) {
        case STATE_INITIAL:
            // Initialize all valves to OFF state
            for (uint8_t i = 0; i < sizeof(cycles)/sizeof(cycles[0]); i++) {
                Reset_GPIO_Pins(cycles[i].pins, cycles[i].pin_count);
            }
            state = STATE_RUN_CYCLE;
            break;

        case STATE_RUN_CYCLE:
            if (cycle_end_time == 0) {
                // Start current cycle
                Set_Valves(cycles[current_cycle].pins, cycles[current_cycle].pin_count);
                cycle_end_time = current_time + cycles[current_cycle].duration_ms;
            }
            else if (current_time >= cycle_end_time) {
                // Cycle completed
                Reset_GPIO_Pins(cycles[current_cycle].pins, cycles[current_cycle].pin_count);

                if (current_cycle < sizeof(cycles)/sizeof(cycles[0]) - 1) {
                    // More cycles to run
                    delay_end_time = current_time + switching_delay;
                    state = STATE_INTER_CYCLE_DELAY;
                }
                else {
                    // All cycles completed
                    state = STATE_COMPLETE;
                }
                cycle_end_time = 0;
            }
            break;

        case STATE_INTER_CYCLE_DELAY:
            if (current_time >= delay_end_time) {
                // Move to next cycle
                current_cycle++;
                state = STATE_RUN_CYCLE;
            }
            break;

        case STATE_COMPLETE:
            // Sequence completed - do nothing
            break;
    }
}
// drain cycle
void Draincycle(void) {
    // Define only Cycle 1
    static const uint32_t cycle1_pins[] = {GPIO_PIN_13, GPIO_PIN_4, GPIO_PIN_9};

    enum {
        STATE_INITIAL,
        STATE_RUN_CYCLE,
        STATE_COMPLETE
    } state = STATE_INITIAL;

    static uint32_t cycle_end_time = 0;
    const uint32_t cycle_duration = 60000;  // 30 seconds

    while (1) {
        uint32_t current_time = HAL_GetTick();

        switch (state) {
            case STATE_INITIAL:
                // Initialize valves to OFF state
                Reset_GPIO_Pins(cycle1_pins, sizeof(cycle1_pins)/sizeof(cycle1_pins[0]));
                state = STATE_RUN_CYCLE;
                break;

            case STATE_RUN_CYCLE:
                if (cycle_end_time == 0) {
                    // Start Cycle 1
                    Set_Valves(cycle1_pins, sizeof(cycle1_pins)/sizeof(cycle1_pins[0]));
                    cycle_end_time = current_time + cycle_duration;
                }
                else if (current_time >= cycle_end_time) {
                    // Cycle 1 completed
                    Reset_GPIO_Pins(cycle1_pins, sizeof(cycle1_pins)/sizeof(cycle1_pins[0]));
                    state = STATE_COMPLETE;
                    cycle_end_time = 0;
                }
                break;

            case STATE_COMPLETE:
                // Do nothing - stays in completed state
                break;
        }
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_ADC1_Init();

	/* USER CODE BEGIN 2 */

	 HAL_UART_Receive_DMA(&huart2, rx_buffer, sizeof(rx_buffer));
	// xTaskCreate(WeightMeasurementTask, "WeightMeasurement", 1024, NULL,
	//tskIDLE_PRIORITY + 1, NULL);
	//xTaskCreate(ValveOperatingTask, "ValveOperatingTask", 512, NULL, 1,NULL);
	//vTaskStartScheduler();
	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* definition and creation of myTask02 */
	osThreadDef(myTask02, StartTask02, osPriorityNormal, 0, 1024);
	myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	//osKernelStart();

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_12, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD,
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
					| GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0
					| GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
					| GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_RESET);

	/*Configure GPIO pin : PA0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PA1 PA12 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PD8 PD9 PD10 PD11
	 PD12 PD13 PD14 PD15
	 PD0 PD1 PD2 PD3
	 PD4 PD5 PD6 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
			| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0
			| GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5
			| GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument) {
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	/*
	 int32_t offset = 0;
	 float scale = NAN;
	 float last_weight = 0.0;
	 int weight = 0;
	 int get_wieght = 0;
	 float density = 1.0;
	 float volume_ml;

	 FLASH_FlushCaches();
	 // Read saved offset and scale from Flash
	 Flash_ReadData(FLASH_USER_START_ADDR, &offset, sizeof(offset));
	 Flash_ReadData(FLASH_USER_START_ADDR + sizeof(offset), &scale,
	 sizeof(scale));

	 if (offset == -1 || isnan(scale)) {
	 snprintf(buffer3, sizeof(buffer3),"hello1 condtion.\n\r:");
	 HAL_UART_Transmit(&huart1, (uint8_t*) buffer3, strlen(buffer3),
	 HAL_MAX_DELAY);
	 // Perform first-time calibration
	 // printf("First-time calibration needed.\n");
	 //		snprintf(buffer0, sizeof(buffer0),
	 //				"First-time calibration needed.\n\r:");
	 //		HAL_UART_Transmit(&huart1, (uint8_t*) buffer0, strlen(buffer0),
	 //		HAL_MAX_DELAY);

	 //printf("Calibrating offset...\n");
	 //		snprintf(buffer1, sizeof(buffer1), "Calibrating offset...\n\r:");
	 //		HAL_UART_Transmit(&huart1, (uint8_t*) buffer1, strlen(buffer1),
	 //		HAL_MAX_DELAY);

	 osDelay(50);
	 offset = HX711_Read();*/
	/*
	 //printf("Offset: %ld\n", offset);
	 snprintf(buffer2, sizeof(buffer2), "Offset: %ld\n\r:");
	 HAL_UART_Transmit(&huart1, (uint8_t*) buffer2, strlen(buffer2),
	 HAL_MAX_DELAY);

	 //printf("Place known weight (e.g., 2kg) on the scale...\n");
	 snprintf(buffer3, sizeof(buffer3),
	 "Place known weight (e.g., 2kg) on the scale...\n\r:");
	 HAL_UART_Transmit(&huart1, (uint8_t*) buffer3, strlen(buffer3),
	 HAL_MAX_DELAY);*/

//		int32_t raw_with_weight = HX711_Read();
//		scale = (float) (raw_with_weight - offset) / 346.0; // Known weight: 2kg
//		// printf("Scale factor: %.2f\n", scale);
//		snprintf(buffer4, sizeof(buffer4), "Scale factor: %.2f\n\r:");
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer4, strlen(buffer4),
//		HAL_MAX_DELAY);
	// Save offset and scale to Flash
	//	Flash_WriteData(FLASH_USER_START_ADDR, &offset, sizeof(offset));
	//	Flash_WriteData(FLASH_USER_START_ADDR + sizeof(offset), &scale,
	//		sizeof(scale));
	//} else {
	//printf("Offset and scale loaded from Flash.\n");
//		snprintf(buffer5, sizeof(buffer5),
//				"Offset and scale loaded from Flash.\n\r:");
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer5, strlen(buffer5),
//		HAL_MAX_DELAY);
//		//// printf("Offset: %ld, Scale: %.2f\n", offset, scale);
//
//		snprintf(buffer6, sizeof(buffer6), "Offset: %ld\n\r:", offset);
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer6, strlen(buffer6),
//		HAL_MAX_DELAY);
//		snprintf(buffer7, sizeof(buffer7), "Scale: %.2f\n\r:", scale);
//		HAL_UART_Transmit(&huart1, (uint8_t*) buffer7, strlen(buffer7),
//		HAL_MAX_DELAY);
	//}
//
	WeightMeasurementTask();
	//Water_pressure_data();
	for (;;) {
		/*
		 weight = HX711_GetWeight(offset, scale);
		 get_wieght=weight;

		 if (weight > 3000.0) {
		 snprintf(buffer, sizeof(buffer),
		 "Overload! Max weight is 2kg.\n\r");
		 HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer),
		 HAL_MAX_DELAY);
		 } else {
		 snprintf(buffer, sizeof(buffer),"                                                   Weight: %d g\n\r",get_wieght);
		 HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer),HAL_MAX_DELAY);

		 }
		 //snprintf(buffer12, sizeof(buffer12), "                                                   raw_data: %.2f g\n\r", raw_value);
		 //HAL_UART_Transmit(&huart1, (uint8_t *)buffer12, strlen(buffer12), HAL_MAX_DELAY);

		 // Save weight to Flash if changed significantly
		 osDelay(250);*/
	}
}
/* USER CODE END 5 */

/* USER CODE BEGIN Header_StartTask02 */
/**
 * @brief Function implementing the myTask02 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask02 */
void StartTask02(void const *argument) {
	/* USER CODE BEGIN StartTask02 */
	/* Infinite loop */
	//ValveOperatingTask();
	//Rinsecycle();
	for(;;)
		   {
		//


		   }
	/* USER CODE END StartTask02 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
