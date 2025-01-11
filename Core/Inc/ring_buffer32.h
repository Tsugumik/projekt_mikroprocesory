/*
 * ring_buffer32.h
 *
 *  Created on: Jan 11, 2025
 *      Author: Błażej Drozd
 */

#ifndef INC_RING_BUFFER32_H_
#define INC_RING_BUFFER32_H_

#define SENSOR_BUFFER_SIZE 750

#include "stdint.h"

typedef struct {
	// Wskaźnik na tablicę bufora
	uint32_t* buffer;
	// head - gdzie zapisywać dane
	volatile uint16_t head;
	// tail - gdzie czytać dane
	volatile uint16_t tail;
	// rozmiar bufora
	uint16_t size;
} RingBuffer32_t;

/*
 * Inicjalizuje wszystkie bufory kołowe.
 * KONIECZNIE należy wykonać w funkcji main
 * przed wejściem do pętli while.
 */
void ring_buffer32_init_all();

/*
 * Inicjalizuje bufor kołowy, przypisuje do niego
 * tablicę i rozmiar.
 */
void ring_buffer32_init(RingBuffer32_t*, uint32_t*, uint16_t);

/*
 * USART_kbhit() - powinien działać tak samo
 */
uint8_t ring_buffer32_is_empty(RingBuffer32_t*);

/*
 * Sprawdza, czy bufor jest pełny.
 * Obliczamy jaka będzie pozycja head po dodaniu jednego elementu.
 * Dzięki operatorowi %, head wróci na początek bufora, gdy
 * przekroczy rozmiar bufora, czyli tworzy efekt "zawijania".
 */
uint8_t ring_buffer32_is_full(RingBuffer32_t*);

// Dodaje bajt do bufora
uint8_t ring_buffer32_put(RingBuffer32_t*, uint32_t);

// Pobiera bajt z bufora
uint8_t ring_buffer32_get(RingBuffer32_t*, uint32_t*);

#endif /* INC_RING_BUFFER32_H_ */
