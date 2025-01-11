/*
 * ring_buffer.c
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
 */

#include "ring_buffer.h"

uint8_t uart_rx_buff[UART_RX_BUFFER_SIZE];
uint8_t uart_tx_buff[UART_TX_BUFFER_SIZE];

RingBuffer_t UART_rx_ring_buffer;
RingBuffer_t UART_tx_ring_buffer;

void ring_buffer_init_all() {
	ring_buffer_init(&UART_rx_ring_buffer, uart_rx_buff, UART_RX_BUFFER_SIZE);
	ring_buffer_init(&UART_tx_ring_buffer, uart_tx_buff, UART_TX_BUFFER_SIZE);
}

void ring_buffer_init(RingBuffer_t* rb, uint8_t* buffer, uint16_t size) {
	rb->buffer = buffer;
	rb->head = 0;
	rb->tail = 0;
	rb->size = size;
}

uint8_t ring_buffer_is_empty(RingBuffer_t* rb) {
	return rb->head == rb->tail;
}

uint8_t ring_buffer_is_full(RingBuffer_t* rb) {
	return ((rb->head + 1) % rb->size) == rb->tail;
}

uint8_t ring_buffer_put(RingBuffer_t* rb, uint8_t data) {
	if(ring_buffer_is_full(rb)) {
		rb->tail = (rb->tail + 1) % rb->size;
	}

	rb->buffer[rb->head] = data;
	rb->head = (rb->head + 1) % rb->size;

	return 1;
}


uint8_t ring_buffer_get(RingBuffer_t* rb, uint8_t* data) {
	if(ring_buffer_is_empty(rb)) {
		return 0;
	}

	*data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % rb->size;

	return 1;
}
