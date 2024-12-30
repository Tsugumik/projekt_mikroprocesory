/*
 * AHT20.c
 *
 *  Created on: Dec 29, 2024
 *      Author: Błażej Drozd - Bydgoszcz University of Science and Technology
 *      License: MIT
 */

#include "AHT20.h"

extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

volatile static AHT20_MainState state = AHT20_STATE_JUST_TURNED_ON;
volatile uint32_t TIM10_time_elapsed = 0;
volatile uint32_t TIM11_time_elapsed = 0;
volatile uint32_t last_measurement_time = 0;
volatile uint8_t aht20_i2c_transfer_complete = 0;

uint32_t read_interval = 1000;

float temperature;
float humidity;

uint8_t get_status_cmd[] = {AHT20_CMD_GET_STATUS};
uint8_t init_cmd[] = {AHT20_CMD_INIT, 0x08, 0x00};
uint8_t measure_cmd[] = {AHT20_CMD_TRIGGER_MEASUREMENT, 0x33, 0x00};

uint8_t rx_buffer[6];

void AHT20_MainStateMachine() {
	static uint8_t status;
	switch (state) {
		case AHT20_STATE_JUST_TURNED_ON:
			state = AHT20_STATE_WAIT_AFTER_TURN_ON;
			TIM10_time_elapsed = 0;
			HAL_TIM_Base_Start_IT(&htim10);
			break;
		case AHT20_STATE_WAIT_AFTER_TURN_ON:
			if (TIM10_time_elapsed >= 40) {
				HAL_TIM_Base_Stop_IT(&htim10);
				aht20_i2c_transfer_complete = 0;
				HAL_I2C_Master_Receive_IT(&hi2c1, AHT20_DEV_ADDRESS_RECEIVE, &status, 1);
				state = AHT20_STATE_CHECK_STATUS;
			}
			break;
		case AHT20_STATE_CHECK_STATUS:
			if (aht20_i2c_transfer_complete) {
				aht20_i2c_transfer_complete = 0;
				if ((status & 0x08) == 0) {
					aht20_i2c_transfer_complete = 0;
					HAL_I2C_Master_Transmit_IT(&hi2c1, AHT20_DEV_ADDRESS_TRANSMIT, init_cmd, 3);
					state = AHT20_STATE_WAIT_FOR_INIT_COMPLETE;
				} else {
					state = AHT20_STATE_IDLE;
				}
			}
			break;
		case AHT20_STATE_WAIT_FOR_INIT_COMPLETE:
			if (aht20_i2c_transfer_complete) {
				aht20_i2c_transfer_complete = 0;
				TIM10_time_elapsed = 0;
				HAL_TIM_Base_Start_IT(&htim10);
				state = AHT20_STATE_WAIT_AFTER_INIT;
			}
			break;
		case AHT20_STATE_WAIT_AFTER_INIT:
			if (TIM10_time_elapsed >= 10) {
				HAL_TIM_Base_Stop_IT(&htim10);
				state = AHT20_STATE_IDLE;
			}
			break;
		case AHT20_STATE_IDLE:
			if(TIM11_time_elapsed - last_measurement_time >= read_interval) {
				last_measurement_time = TIM11_time_elapsed;
				aht20_i2c_transfer_complete = 0;
				HAL_I2C_Master_Transmit_IT(&hi2c1, AHT20_DEV_ADDRESS_TRANSMIT, measure_cmd, 3);
				TIM10_time_elapsed = 0;
				HAL_TIM_Base_Start_IT(&htim10);
				state = AHT20_STATE_WAIT_FOR_MEASURE_COMPLETE;
			}
			break;
		case AHT20_STATE_WAIT_FOR_MEASURE_COMPLETE:
			if (TIM10_time_elapsed >= 80) {
				HAL_TIM_Base_Stop_IT(&htim10);
				aht20_i2c_transfer_complete = 0;
				HAL_I2C_Master_Receive_IT(&hi2c1, AHT20_DEV_ADDRESS_RECEIVE, rx_buffer, 6);
				state = AHT20_STATE_PROCESS_DATA;
			}
			break;
		case AHT20_STATE_PROCESS_DATA:
			if(aht20_i2c_transfer_complete){
				aht20_i2c_transfer_complete = 0;

				uint32_t raw_humidity_20bit = (rx_buffer[1]) << 12 | (rx_buffer[2]) << 4 | (rx_buffer[3]) >> 4;
				uint32_t raw_temperature_20bit = (rx_buffer[3] & 0x0F) << 16 | (rx_buffer[4]) << 8 | (rx_buffer[5]);

				SCREEN_CalculateValues(&raw_temperature_20bit, &raw_humidity_20bit);

				state = AHT20_STATE_IDLE;
			}
			break;
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
	if(hi2c == &hi2c1) {
		aht20_i2c_transfer_complete = 1;
	}
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
	if(hi2c == &hi2c1) {
		aht20_i2c_transfer_complete = 1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM10) {
	  TIM10_time_elapsed++;
  } else if(htim->Instance == TIM11) {
	  TIM11_time_elapsed++;
  }
}

