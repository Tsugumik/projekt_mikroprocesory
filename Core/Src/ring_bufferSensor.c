/*
 * ring_buffer32.c
 *
 *  Created on: Jan 11, 2025
 *      Author: drozd
 */

#include "ring_bufferSensor.h"

Sensor_RawData_t sensor_buffer[SENSOR_BUFFER_SIZE];

RingBufferSensor_RawData_t SENSOR_ring_buffer;

void ring_bufferSensor_init_all() {
	ring_bufferSensor_init(&SENSOR_ring_buffer, sensor_buffer, SENSOR_BUFFER_SIZE);
}

void ring_bufferSensor_init(RingBufferSensor_RawData_t* rb, Sensor_RawData_t* buffer, uint16_t size) {
	rb->buffer = buffer;
	rb->head = 0;
	rb->tail = 0;
	rb->size = size;
}

uint8_t ring_bufferSensor_is_empty(RingBufferSensor_RawData_t* rb) {
	return rb->head == rb->tail;
}

uint8_t ring_bufferSensor_is_full(RingBufferSensor_RawData_t* rb) {
	return ((rb->head + 1) % rb->size) == rb->tail;
}

uint8_t ring_bufferSensor_put(RingBufferSensor_RawData_t* rb, Sensor_RawData_t data) {
	if(ring_bufferSensor_is_full(rb)) {
		rb->tail = (rb->tail + 1) % rb->size;
	}

	rb->buffer[rb->head] = data;
	rb->head = (rb->head + 1) % rb->size;

	return 1;
}

uint8_t ring_bufferSensor_get(RingBufferSensor_RawData_t* rb, Sensor_RawData_t* data) {
	if(ring_bufferSensor_is_empty(rb)) {
		return 0;
	}

	*data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % rb->size;

	return 1;
}
