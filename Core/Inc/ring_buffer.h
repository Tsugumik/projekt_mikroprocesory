/*
 * ring_buffer.h
 *
 *  Created on: Nov 3, 2024
 *      Author: drozd
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_
#define BUFFER_SIZE 750
#include "stdint.h"

typedef struct {
	uint8_t buffer[BUFFER_SIZE];
	uint16_t head;
	uint16_t tail;
	uint16_t size;
} RingBuffer_t;

void ring_buffer_init(RingBuffer_t*);

// Sprawdza, czy bufor jest pusty
uint8_t ring_buffer_is_empty(RingBuffer_t*);

/*
 * Sprawdza, czy bufor jest pełny.
 * Obliczamy jaka będzie pozycja head po dodaniu jednego elementu.
 * Dzięki operatorowi %, head wróci na początek bufora, gdy
 * przekroczy rozmiar bufora, czyli tworzy efekt "zawijania".
 */
uint8_t ring_buffer_is_full(RingBuffer_t*);

// Dodaje bajt do bufora
uint8_t ring_buffer_put(RingBuffer_t*, uint8_t);

// Pobiera bajt z bufora
uint8_t ring_buffer_get(RingBuffer_t*, uint8_t*);

#endif /* INC_RING_BUFFER_H_ */
