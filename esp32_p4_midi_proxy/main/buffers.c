/*
 * Static Buffer Management Implementation
 */

#include "buffers.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "buffers";

//--------------------------------------------------------------------+
// Global Static Buffers
//--------------------------------------------------------------------+

usb_host_buffer_t g_usb_host_rx_buffer;
usb_device_buffer_t g_usb_device_tx_buffer;
hercules_state_t g_hercules_state;

// MIDI queue static allocation
QueueHandle_t g_midi_queue = NULL;
StaticQueue_t g_midi_queue_struct;
uint8_t g_midi_queue_storage[MIDI_QUEUE_DEPTH * sizeof(midi_message_t)];

//--------------------------------------------------------------------+
// Initialization
//--------------------------------------------------------------------+

void buffers_init(void)
{
    ESP_LOGI(TAG, "Initializing static buffers...");

    // Clear all buffers
    memset(&g_usb_host_rx_buffer, 0, sizeof(g_usb_host_rx_buffer));
    memset(&g_usb_device_tx_buffer, 0, sizeof(g_usb_device_tx_buffer));
    memset(&g_hercules_state, 0, sizeof(g_hercules_state));

    // Create MIDI queue with static allocation
    g_midi_queue = xQueueCreateStatic(
        MIDI_QUEUE_DEPTH,
        sizeof(midi_message_t),
        g_midi_queue_storage,
        &g_midi_queue_struct
    );

    if (g_midi_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create MIDI queue");
        return;
    }

    ESP_LOGI(TAG, "Static buffers initialized:");
    ESP_LOGI(TAG, "  USB Host RX:   %d bytes", USB_HOST_RX_BUFFER_SIZE);
    ESP_LOGI(TAG, "  USB Device TX: %d bytes", USB_DEVICE_TX_BUFFER_SIZE);
    ESP_LOGI(TAG, "  MIDI Queue:    %d messages", MIDI_QUEUE_DEPTH);
    ESP_LOGI(TAG, "  Hercules:      %d bytes", HERCULES_REPORT_SIZE);
}

//--------------------------------------------------------------------+
// USB Host Buffer Management
//--------------------------------------------------------------------+

void usb_host_buffer_reset(void)
{
    g_usb_host_rx_buffer.length = 0;
    g_usb_host_rx_buffer.valid = false;
}

//--------------------------------------------------------------------+
// USB Device Buffer Management
//--------------------------------------------------------------------+

uint16_t usb_device_buffer_available(void)
{
    if (g_usb_device_tx_buffer.write_pos >= g_usb_device_tx_buffer.read_pos) {
        return USB_DEVICE_TX_BUFFER_SIZE -
               (g_usb_device_tx_buffer.write_pos - g_usb_device_tx_buffer.read_pos);
    } else {
        return g_usb_device_tx_buffer.read_pos - g_usb_device_tx_buffer.write_pos;
    }
}

bool usb_device_buffer_write(const uint8_t *data, uint16_t length)
{
    if (length == 0 || data == NULL) {
        return false;
    }

    if (usb_device_buffer_available() < length) {
        ESP_LOGW(TAG, "USB Device TX buffer full, dropping %d bytes", length);
        return false;
    }

    for (uint16_t i = 0; i < length; i++) {
        g_usb_device_tx_buffer.data[g_usb_device_tx_buffer.write_pos] = data[i];
        g_usb_device_tx_buffer.write_pos =
            (g_usb_device_tx_buffer.write_pos + 1) % USB_DEVICE_TX_BUFFER_SIZE;
    }

    g_usb_device_tx_buffer.length += length;
    return true;
}

uint16_t usb_device_buffer_read(uint8_t *data, uint16_t max_length)
{
    if (max_length == 0 || data == NULL) {
        return 0;
    }

    uint16_t available = g_usb_device_tx_buffer.length;
    uint16_t to_read = (available < max_length) ? available : max_length;

    for (uint16_t i = 0; i < to_read; i++) {
        data[i] = g_usb_device_tx_buffer.data[g_usb_device_tx_buffer.read_pos];
        g_usb_device_tx_buffer.read_pos =
            (g_usb_device_tx_buffer.read_pos + 1) % USB_DEVICE_TX_BUFFER_SIZE;
    }

    g_usb_device_tx_buffer.length -= to_read;
    return to_read;
}

void usb_device_buffer_clear(void)
{
    g_usb_device_tx_buffer.length = 0;
    g_usb_device_tx_buffer.write_pos = 0;
    g_usb_device_tx_buffer.read_pos = 0;
}

//--------------------------------------------------------------------+
// Hercules State Management
//--------------------------------------------------------------------+

bool hercules_state_has_changed(void)
{
    if (!g_hercules_state.initialized) {
        return true;
    }

    return memcmp(g_hercules_state.current, g_hercules_state.previous,
                  HERCULES_REPORT_SIZE) != 0;
}

void hercules_state_update(void)
{
    memcpy(g_hercules_state.previous, g_hercules_state.current,
           HERCULES_REPORT_SIZE);
    g_hercules_state.initialized = true;
}
