/*
 * AHT20.h
 *
 *  Created on: Dec 29, 2024
 *      Author: Błażej Drozd - Bydgoszcz University of Science and Technology
 *      License: MIT
 */

#ifndef AHT20_H_
#define AHT20_H_

#include "stm32f4xx_hal.h"
#include "screen.h"
#include "ring_bufferSensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AHT20_DEV_ADDRESS_7BIT 0x38
#define AHT20_DEV_ADDRESS_TRANSMIT (AHT20_DEV_ADDRESS_7BIT << 1)      // 0x70
#define AHT20_DEV_ADDRESS_RECEIVE ((AHT20_DEV_ADDRESS_7BIT << 1) | 0x01) // 0x71

#define AHT20_CMD_INIT 0xBE
#define AHT20_CMD_TRIGGER_MEASUREMENT 0xAC
#define AHT20_CMD_SOFT_RESET 0xBA
#define AHT20_CMD_GET_STATUS 0x71

typedef enum {
    AHT20_STATE_JUST_TURNED_ON,        // Stan początkowy, po włączeniu zasilania
    AHT20_STATE_WAIT_AFTER_TURN_ON,    // Oczekiwanie po włączeniu (40ms)
    AHT20_STATE_CHECK_STATUS,           // Odbiór statusu i sprawdzenie bitu kalibracji
    AHT20_STATE_WAIT_FOR_INIT_COMPLETE, // Oczekiwanie na zakończenie transmisji inicjalizującej
    AHT20_STATE_WAIT_AFTER_INIT,       // Oczekiwanie po inicjalizacji (10ms)
    AHT20_STATE_IDLE,                 // Czujnik gotowy do pomiarów
    AHT20_STATE_WAIT_FOR_MEASURE_COMPLETE, // Oczekiwanie na zakończenie pomiaru (80ms)
    AHT20_STATE_PROCESS_DATA,         // Przetwarzanie odebranych danych
} AHT20_MainState;

void AHT20_MainStateMachine();

#endif /* AHT20_H_ */
