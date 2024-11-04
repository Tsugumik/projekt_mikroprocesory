/*
 * ring_buffer.c
 *
 *  Created on: Nov 3, 2024
 *      Author: drozd
 */

#include "ring_buffer.h"

// Inicjalizuje bufor
void ring_buffer_init(RingBuffer_t* rb) {
	rb->head = 0;
	rb->tail = 0;
	rb->size = BUFFER_SIZE;
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
