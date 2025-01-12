/*
 * ring_buffer32.h
 *
 *  Created on: Jan 11, 2025
 *      Author: Błażej Drozd
 */

#ifndef INC_RING_BUFFERSENSOR_H_
#define INC_RING_BUFFERSENSOR_H_

#define SENSOR_BUFFER_SIZE 750

#include "stdint.h"

typedef struct {
	uint32_t temperature;
	uint32_t humidity;
} Sensor_RawData_t;

typedef struct {
	// Wskaźnik na tablicę bufora
	Sensor_RawData_t* buffer;
	// head - gdzie zapisywać dane
	volatile uint16_t head;
	// tail - gdzie czytać dane
	volatile uint16_t tail;
	// rozmiar bufora
	uint16_t size;
} RingBufferSensor_RawData_t;

/*
 * Inicjalizuje wszystkie bufory kołowe.
 * KONIECZNIE należy wykonać w funkcji main
 * przed wejściem do pętli while.
 */
void ring_bufferSensor_init_all();

/*
 * Inicjalizuje bufor kołowy, przypisuje do niego
 * tablicę i rozmiar.
 */
void ring_bufferSensor_init(RingBufferSensor_RawData_t*, Sensor_RawData_t*, uint16_t);

/*
 * USART_kbhit() - powinien działać tak samo
 */
uint8_t ring_bufferSensor_is_empty(RingBufferSensor_RawData_t*);

/*
 * Sprawdza, czy bufor jest pełny.
 * Obliczamy jaka będzie pozycja head po dodaniu jednego elementu.
 * Dzięki operatorowi %, head wróci na początek bufora, gdy
 * przekroczy rozmiar bufora, czyli tworzy efekt "zawijania".
 */
uint8_t ring_bufferSensor_is_full(RingBufferSensor_RawData_t*);

// Dodaje bajt do bufora
uint8_t ring_bufferSensor_put(RingBufferSensor_RawData_t*, Sensor_RawData_t);

// Pobiera bajt z bufora
uint8_t ring_bufferSensor_get(RingBufferSensor_RawData_t*, Sensor_RawData_t*);

uint8_t ring_bufferSensor_get_latest(RingBufferSensor_RawData_t*, Sensor_RawData_t*);

uint8_t ring_bufferSensor_get_at_index(RingBufferSensor_RawData_t*, uint16_t, Sensor_RawData_t*);

uint16_t ring_bufferSensor_get_oldest_index(RingBufferSensor_RawData_t*);

#endif /* INC_RING_BUFFERSENSOR_H_ */
