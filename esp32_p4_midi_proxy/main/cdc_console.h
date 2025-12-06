/*
 * CDC Debug Console
 *
 * Handles debug commands via USB CDC serial
 */

#ifndef CDC_CONSOLE_H
#define CDC_CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize CDC console
 */
void cdc_console_init(void);

/**
 * Process incoming CDC data
 * @param data Received data buffer
 * @param len Length of received data
 */
void cdc_console_process(const uint8_t *data, uint16_t len);

/**
 * Print formatted string to CDC console
 * @param format printf-style format string
 */
void cdc_console_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));

/**
 * Print help message
 */
void cdc_console_print_help(void);

#ifdef __cplusplus
}
#endif

#endif // CDC_CONSOLE_H
