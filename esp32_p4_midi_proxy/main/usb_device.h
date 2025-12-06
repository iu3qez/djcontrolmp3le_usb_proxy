/*
 * USB Device Header
 */

#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "midi_types.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize USB Device (OTG2)
 */
void usb_device_init(void);

/**
 * USB Device task (should run in FreeRTOS task)
 */
void usb_device_task(void *pvParameters);

/**
 * Send MIDI message to USB
 * @param msg MIDI message to send
 * @return true if sent successfully
 */
bool midi_send(const midi_message_t *msg);

/**
 * Printf to CDC serial
 * @param format printf-style format string
 */
void cdc_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // USB_DEVICE_H
