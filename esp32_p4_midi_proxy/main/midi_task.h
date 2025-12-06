/*
 * MIDI Task
 *
 * FreeRTOS task that consumes MIDI messages from queue and sends via USB
 */

#ifndef MIDI_TASK_H
#define MIDI_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MIDI TX task - consumes from MIDI queue and sends via USB Device
 */
void midi_tx_task(void *pvParameters);

/**
 * Get MIDI queue statistics
 */
typedef struct {
    uint32_t messages_sent;
    uint32_t messages_dropped;
    uint32_t queue_high_water;
} midi_stats_t;

void midi_get_stats(midi_stats_t *stats);
void midi_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif // MIDI_TASK_H
