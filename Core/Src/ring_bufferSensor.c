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
    if (ring_bufferSensor_is_full(rb)) {
        rb->tail = (rb->tail + 1) % rb->size;
    }

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->size;

    return 1;
}

uint8_t ring_bufferSensor_get(RingBufferSensor_RawData_t* rb, Sensor_RawData_t* data) {
    if (ring_bufferSensor_is_empty(rb)) {
        return 0;
    }

    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;

    return 1;
}

// Pobieranie najnowszej pozycji z bufora (bieżącej)
uint8_t ring_bufferSensor_get_latest(RingBufferSensor_RawData_t* rb, Sensor_RawData_t* data) {
    if (ring_bufferSensor_is_empty(rb)) {
        return 0;
    }

    uint16_t latest_index = (rb->head == 0) ? (rb->size - 1) : (rb->head - 1);
    *data = rb->buffer[latest_index];
    return 1;
}

// Pobieranie wartości z bufora o podanym indeksie (względem początku bufora kołowego)
uint8_t ring_bufferSensor_get_at_index(RingBufferSensor_RawData_t* rb, uint16_t index, Sensor_RawData_t* data) {
    if (ring_bufferSensor_is_empty(rb) || index >= rb->size) {
        return 0;
    }

    uint16_t actual_index = (rb->tail + index) % rb->size;
    *data = rb->buffer[actual_index];
    return 1;
}

// Pobieranie numeru indeksu, na którym znajduje się obecnie najstarsza wartość (tail)
uint16_t ring_bufferSensor_get_oldest_index(RingBufferSensor_RawData_t* rb) {
    return rb->tail;
}

uint8_t ring_bufferSensor_get_latest_index(RingBufferSensor_RawData_t* rb, uint16_t* out) {
    if (ring_bufferSensor_is_empty(rb)) {
        return 0;
    }

    *out = (rb->head == 0) ? (rb->size - 1) : (rb->head - 1);
    return 1;
}

uint8_t ring_bufferSensor_can_get_range(RingBufferSensor_RawData_t* rb, uint16_t index_from, uint16_t index_to) {
    if (ring_bufferSensor_is_empty(rb)) {
        return 0; // Błąd: bufor pusty
    }

    if (index_from > index_to) {
        return 0; // Błąd: nieprawidłowy zakres (index_from > index_to)
    }

    if (index_from >= rb->size || index_to >= rb->size) {
        return 0; // Błąd: indeks poza zakresem bufora
    }

    uint16_t count = index_to - index_from + 1;
    uint16_t available_data = (rb->head - rb->tail + rb->size) % rb->size;

    if (count > available_data) {
        return 0; // Błąd: za mało danych w buforze
    }

    return 1; // Można pobrać dane
}
