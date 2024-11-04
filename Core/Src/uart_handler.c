/*
 * uart_handler.c
 *
 *  Created on: Nov 3, 2024
 *      Author: drozd
 */

#include "uart_handler.h"

uint8_t UART_rx_temp;
uint8_t UART_tx_temp;

uint32_t test_counter = 0;

volatile uint8_t UART_tx_in_progress = 0;

extern RingBuffer_t UART_rx_ring_buffer;
extern RingBuffer_t UART_tx_ring_buffer;
extern RingBuffer_t Sensor_ring_buffer;

/*
 * Obsługa odbierania danych
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance != USART2) return;

	// TESTOWANIE
	char txtbuff[100];
	test_counter++;
	snprintf(txtbuff, sizeof(txtbuff), "STM32 RX COUNT: %lu\r\n", test_counter);
	// TESTOWANIE

	UART_SendText(txtbuff);

	ring_buffer_put(&UART_rx_ring_buffer, UART_rx_temp);

	HAL_UART_Receive_IT(&huart2, &UART_rx_temp, 1);
}

/*
 * Obsługa wysyłania danych
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance != USART2) return;

	if(ring_buffer_is_empty(&UART_tx_ring_buffer)) {
		UART_tx_in_progress = 0;
		return;
	}

	ring_buffer_get(&UART_tx_ring_buffer, &UART_tx_temp);

	HAL_UART_Transmit_IT(&huart2, &UART_tx_temp, 1);
}

/*
 * Funkcja do wysyłania pojedyńczych bajtów
 */
void UART_SendData(uint8_t* data, uint16_t length) {
	for(uint16_t i = 0; i < length; i++) {
		ring_buffer_put(&UART_tx_ring_buffer, data[i]);
	}

	if(UART_tx_in_progress) return;

	UART_tx_in_progress = 1;

	ring_buffer_get(&UART_tx_ring_buffer, &UART_tx_temp);
	HAL_UART_Transmit_IT(&huart2, &UART_tx_temp, 1);
}

/*
 * Funkcja do wysyłania komunikatów tekstowych (bez użycia ramki)
 */
void UART_SendText(const char* text) {
	size_t length = strlen(text);

	UART_SendData((uint8_t*)text, length);
}
