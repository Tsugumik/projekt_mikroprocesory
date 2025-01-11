/*
 * ring_buffer32.c
 *
 *  Created on: Jan 11, 2025
 *      Author: drozd
 */

#include "ring_buffer32.h"

uint32_t sensor_buffer[SENSOR_BUFFER_SIZE];

RingBuffer32_t SENSOR_ring_buffer;

void ring_buffer32_init_all() {
	ring_buffer32_init(&SENSOR_ring_buffer, sensor_buffer, SENSOR_BUFFER_SIZE);
}

void ring_buffer32_init(RingBuffer32_t* rb, uint32_t* buffer, uint16_t size) {
	rb->buffer = buffer;
	rb->head = 0;
	rb->tail = 0;
	rb->size = size;
}

uint8_t ring_buffer32_is_empty(RingBuffer32_t* rb) {
	return rb->head == rb->tail;
}

uint8_t ring_buffer32_is_full(RingBuffer32_t* rb) {
	return ((rb->head + 1) % rb->size) == rb->tail;
}

uint8_t ring_buffer32_put(RingBuffer32_t* rb, uint32_t data) {
	if(ring_buffer32_is_full(rb)) {
		rb->tail = (rb->tail + 1) % rb->size;
	}

	rb->buffer[rb->head] = data;
	rb->head = (rb->head + 1) % rb->size;

	return 1;
}

uint8_t ring_buffer32_get(RingBuffer32_t* rb, uint32_t* data) {
	if(ring_buffer32_is_empty(rb)) {
		return 0;
	}

	*data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % rb->size;

	return 1;
}
