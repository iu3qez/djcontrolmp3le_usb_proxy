/*
 * MIDI Task Implementation
 */

#include "midi_task.h"
#include "midi_types.h"
#include "usb_device.h"
#include "buffers.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "midi_task";

// Statistics
static midi_stats_t stats = {0};

void midi_tx_task(void *pvParameters)
{
    (void)pvParameters;

    ESP_LOGI(TAG, "MIDI TX task started");
    midi_message_t msg;

    while (1) {
        // Wait for MIDI message from queue (block indefinitely)
        if (xQueueReceive(g_midi_queue, &msg, portMAX_DELAY) == pdTRUE) {

            // Send MIDI message via USB Device
            if (midi_send(&msg)) {
                stats.messages_sent++;

                #if DEBUG_MIDI_LOG
                ESP_LOGD(TAG, "MIDI TX: %02x %02x %02x",
                         msg.status, msg.data1, msg.data2);
                #endif
            } else {
                stats.messages_dropped++;
                ESP_LOGW(TAG, "Failed to send MIDI message (USB not ready?)");
            }

            // Update high water mark
            UBaseType_t waiting = uxQueueMessagesWaiting(g_midi_queue);
            if (waiting > stats.queue_high_water) {
                stats.queue_high_water = waiting;
            }
        }
    }
}

void midi_get_stats(midi_stats_t *out_stats)
{
    if (out_stats) {
        *out_stats = stats;
    }
}

void midi_reset_stats(void)
{
    stats.messages_sent = 0;
    stats.messages_dropped = 0;
    stats.queue_high_water = 0;
    ESP_LOGI(TAG, "Statistics reset");
}
