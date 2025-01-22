# Projekt: Komunikacja STM32 z PC i obsługa czujnika AHT20

## Opis projektu

Projekt został zrealizowany w ramach przedmiotu **Mikroprocesory**. Wykorzystano płytkę **STM32 Nucleo F446RE**, czujnik temperatury i wilgotności **AHT20** oraz wyświetlacz **SSD1306**. Projekt polega na:

- Obsłudze komunikacji między mikroprocesorem a komputerem za pomocą protokołu UART z wykorzystaniem przerwań i buforów kołowych.
- Implementacji protokołu komunikacyjnego umożliwiającego przesyłanie danych, adresowanie pakietów oraz weryfikację poprawności przesyłanych danych.
- Obsłudze czujnika AHT20 przez interfejs I2C z przerwaniami i zapisywaniu pomiarów do bufora kołowego.
- Wyświetlaniu aktualnych wartości temperatury i wilgotności na ekranie OLED.

## Specyfikacja

- **Płytka:** STM32 Nucleo F446RE
- **Czujnik:** Seed Studio Grove AHT20
- **Wyświetlacz:** OLED SSD1306
- **Sterownik do wyświetlacza:** [Aleksander Alekseev](https://github.com/afiskon/stm32-ssd1306) (licencja MIT)

## Funkcjonalności

1. Komunikacja UART z obsługą przerwań i buforów kołowych.
2. Protokół komunikacyjny umożliwiający:
   - Kodowanie danych w formacie szesnastkowym.
   - Obliczanie i dołączanie sumy kontrolnej CRC-16.
   - Obsługę ramek zwrotnych: `DATA`, `STATUS`, `ERROR`.
3. Obsługa czujnika AHT20:
   - Odczytywanie temperatury i wilgotności co zadany interwał pomiarowy (domyślnie 1000 ms).
   - Archiwizacja pomiarów (do 750 wpisów).
4. Wyświetlanie wartości na ekranie OLED.
5. Obsługa komend:
   - Włączanie/wyłączanie wyświetlacza.
   - Przełączanie jednostek temperatury (Celsjusz, Fahrenheit, Kelvin).
   - Pobieranie danych bieżących i archiwalnych.
   - Ustawianie interwału pomiarowego.

## Protokół komunikacyjny

### Struktura ramki

| Początek | ID Nadawcy | ID Odbiorcy | Długość danych | Dane | Suma kontrolna | Koniec |
|----------|------------|-------------|----------------|------|----------------|-------|
| `{` (0x7B) | Kodowanie szesnastkowe (2 znaki) | Kodowanie szesnastkowe (2 znaki) | Kodowanie szesnastkowe (2 znaki) | Wybrane znaki ASCII (0-200) | Kodowanie szesnastkowe (4 znaki) | `}` (0x7D) |

### Dozwolone dane w polu "Dane"
- Wielkie litery `A-Z` (0x41-0x5A)
- Cyfry `0-9` (0x30-0x39)
- Nawiasy `(` (0x28) i `)` (0x29)
- Przecinek `,` (0x2C)

### Przykładowa ramka
```
{2AFF0CSETINTERVAL(927C0)92F6}
```

## Komendy

| Komenda                    | Argumenty                     | Opis                                              |
|----------------------------|-------------------------------|--------------------------------------------------|
| `SETSCREEN`                | `0` lub `1`                  | Włącza (1) lub wyłącza (0) wyświetlacz.          |
| `SETSCREENTEMPUNIT`        | `0` (C), `1` (F), `2` (K)    | Ustawia jednostkę temperatury na wyświetlaczu.   |
| `SETRETURNTEMPUNIT`        | `0` (C), `1` (F), `2` (K)    | Ustawia jednostkę temperatury dla danych zwrotnych. |
| `GETRETURNTEMPUNIT`        | Brak                          | Zwraca aktualnie ustawioną jednostkę temperatury.|
| `SETINTERVAL`              | `0x0000` - `0xFFFFFFFF`          | Ustawia interwał pomiarowy (w ms).               |
| `GETARCHIVE`               | `IndeksOd`, `IndeksDo`, Typ   | Pobiera dane archiwalne.                         |
| `GETDATA`               | Brak                          | Pobiera dane bieżące.                            |

+ Dodatkowe, do przejrzenia w kodzie projektu.

## Instalacja i uruchomienie
1. Wgraj program na płytkę STM32 za pomocą środowiska programistycznego (np. STM32CubeIDE).
2. Podłącz płytkę do źródła zasilania lub portu USB.
3. Urządzenie automatycznie rozpocznie pomiary i obsługę komunikacji.

## Autor

- **Imię i nazwisko:** Błażej Drozd
- **Prowadzący:** dr inż. Sławomir Bujnowski

## Licencja

Ten projekt jest objęty licencją MIT. Szczegóły znajdują się w pliku `LICENSE` w repozytorium.
