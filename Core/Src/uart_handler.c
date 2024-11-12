/*
 * uart_handler.c
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
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

	// Wstawienie otrzymanych danych do bufora kołowego
	ring_buffer_put(&UART_rx_ring_buffer, UART_rx_temp);

	// Oczekiwanie na kolejne dane
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
 * Funkcja do wysyłania bajtów
 */
void UART_SendData(uint8_t* data, uint16_t length) {
	for(uint16_t i = 0; i < length; i++) {
		__disable_irq();
		ring_buffer_put(&UART_tx_ring_buffer, data[i]);
		__enable_irq();
	}

	__disable_irq();
	if(UART_tx_in_progress) {
		__enable_irq();
		return;
	}

	UART_tx_in_progress = 1;

	ring_buffer_get(&UART_tx_ring_buffer, &UART_tx_temp);

	HAL_UART_Transmit_IT(&huart2, &UART_tx_temp, 1);

	__enable_irq();
}

/*
 * Funkcja do wysyłania komunikatów tekstowych (bez użycia ramki)
 */
void UART_SendText(const char* text) {
	size_t length = strlen(text);

	UART_SendData((uint8_t*)text, length);
}
