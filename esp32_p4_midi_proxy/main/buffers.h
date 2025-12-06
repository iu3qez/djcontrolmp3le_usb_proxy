/*
 * Static Buffer Management
 *
 * All buffers allocated statically (no malloc)
 */

#ifndef BUFFERS_H
#define BUFFERS_H

#include <stdint.h>
#include "config.h"
#include "midi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Static Buffer Structures
//--------------------------------------------------------------------+

// USB Host RX buffer
typedef struct {
    uint8_t data[USB_HOST_RX_BUFFER_SIZE];
    uint16_t length;
    bool valid;
} usb_host_buffer_t;

// USB Device TX buffer
typedef struct {
    uint8_t data[USB_DEVICE_TX_BUFFER_SIZE];
    uint16_t length;
    uint16_t write_pos;
    uint16_t read_pos;
} usb_device_buffer_t;

// Hercules controller state buffer
typedef struct {
    uint8_t current[HERCULES_REPORT_SIZE];
    uint8_t previous[HERCULES_REPORT_SIZE];
    bool initialized;
} hercules_state_t;

//--------------------------------------------------------------------+
// Global Static Buffers
//--------------------------------------------------------------------+

extern usb_host_buffer_t g_usb_host_rx_buffer;
extern usb_device_buffer_t g_usb_device_tx_buffer;
extern hercules_state_t g_hercules_state;

// MIDI message queue (FreeRTOS static allocation)
extern QueueHandle_t g_midi_queue;
extern StaticQueue_t g_midi_queue_struct;
extern uint8_t g_midi_queue_storage[MIDI_QUEUE_DEPTH * sizeof(midi_message_t)];

//--------------------------------------------------------------------+
// Buffer Management Functions
//--------------------------------------------------------------------+

/**
 * Initialize all static buffers
 */
void buffers_init(void);

/**
 * Reset USB host buffer
 */
void usb_host_buffer_reset(void);

/**
 * Get available space in USB device TX buffer
 */
uint16_t usb_device_buffer_available(void);

/**
 * Write data to USB device TX buffer
 */
bool usb_device_buffer_write(const uint8_t *data, uint16_t length);

/**
 * Read data from USB device TX buffer
 */
uint16_t usb_device_buffer_read(uint8_t *data, uint16_t max_length);

/**
 * Clear USB device TX buffer
 */
void usb_device_buffer_clear(void);

/**
 * Check if Hercules state has changed
 */
bool hercules_state_has_changed(void);

/**
 * Update Hercules state (copy current to previous)
 */
void hercules_state_update(void);

#ifdef __cplusplus
}
#endif

#endif // BUFFERS_H
