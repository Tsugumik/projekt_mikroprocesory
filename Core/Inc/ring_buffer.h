/*
 * ring_buffer.h
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#define DEFAULT_BUFFER_SIZE 1000
#define UART_RX_BUFFER_SIZE 3000
#define UART_TX_BUFFER_SIZE 3000

#include "stdint.h"

typedef struct {
	// Wskaźnik na tablicę bufora
	uint8_t* buffer;
	// head - gdzie zapisywać dane
	volatile uint16_t head;
	// tail - gdzie czytać dane
	volatile uint16_t tail;
	// rozmiar bufora
	uint16_t size;
} RingBuffer_t;

/*
 * Inicjalizuje wszystkie bufory kołowe.
 * KONIECZNIE należy wykonać w funkcji main
 * przed wejściem do pętli while.
 */
void ring_buffer_init_all();

/*
 * Inicjalizuje bufor kołowy, przypisuje do niego
 * tablicę i rozmiar.
 */
void ring_buffer_init(RingBuffer_t*, uint8_t*, uint16_t);

/*
 * USART_kbhit() - powinien działać tak samo
 */
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
