/*
 * MIDI Converter Implementation
 */

#include "midi_converter.h"
#include "hercules_protocol.h"
#include "buffers.h"
#include "esp_log.h"
#include "config.h"

static const char *TAG = "midi_converter";

void midi_converter_init(void)
{
    ESP_LOGI(TAG, "MIDI Converter initialized");
    ESP_LOGI(TAG, "  MIDI Channel: %d", MIDI_CHANNEL + 1);
    ESP_LOGI(TAG, "  Controls mapped: %d", HERCULES_CONTROL_COUNT);
}

uint8_t midi_converter_process(const uint8_t *current, const uint8_t *previous)
{
    uint8_t midi_count = 0;

    // Iterate through all controls and detect changes
    for (uint8_t i = 0; i < HERCULES_CONTROL_COUNT; i++) {
        const hercules_control_t *ctrl = &hercules_controls[i];

        uint8_t old_val = previous[ctrl->byte_offset] & ctrl->byte_mask;
        uint8_t new_val = current[ctrl->byte_offset] & ctrl->byte_mask;

        // For buttons, convert to boolean
        if (ctrl->control_type == CONTROL_TYPE_BUTTON) {
            old_val = (old_val > 0) ? 1 : 0;
            new_val = (new_val > 0) ? 1 : 0;
        }

        // Check if value changed
        if (new_val != old_val) {
            midi_message_t msg;

            switch (ctrl->control_type) {
                case CONTROL_TYPE_BUTTON:
                    msg = midi_converter_button_to_note(ctrl->midi_note_or_cc, new_val);
                    ESP_LOGD(TAG, "Button %s: %d -> %d (Note %d %s)",
                             ctrl->name, old_val, new_val, ctrl->midi_note_or_cc,
                             new_val ? "ON" : "OFF");
                    break;

                case CONTROL_TYPE_DIAL:
                    msg = midi_converter_dial_to_cc(ctrl->midi_note_or_cc, new_val);
                    ESP_LOGD(TAG, "Dial %s: %d -> %d (CC %d = %d)",
                             ctrl->name, old_val, new_val, ctrl->midi_note_or_cc, new_val / 2);
                    break;

                case CONTROL_TYPE_JOG:
                    msg = midi_converter_jog_to_cc(ctrl->midi_note_or_cc, old_val, new_val);
                    ESP_LOGD(TAG, "Jog %s: %d -> %d (CC %d = %d)",
                             ctrl->name, old_val, new_val, ctrl->midi_note_or_cc, new_val / 2);
                    break;

                default:
                    continue;
            }

            // Send to MIDI queue
            if (xQueueSend(g_midi_queue, &msg, 0) == pdTRUE) {
                midi_count++;
            } else {
                ESP_LOGW(TAG, "MIDI queue full, dropped message");
            }
        }
    }

    return midi_count;
}

midi_message_t midi_converter_button_to_note(uint8_t note_number, bool is_pressed)
{
    if (is_pressed) {
        // Note On with velocity 100
        return midi_create_note_on(MIDI_CHANNEL, note_number, 100);
    } else {
        // Note Off with velocity 0
        return midi_create_note_off(MIDI_CHANNEL, note_number, 0);
    }
}

midi_message_t midi_converter_dial_to_cc(uint8_t cc_number, uint8_t value)
{
    // MIDI CC values are 0-127, Hercules sends 0-255
    // Divide by 2 to convert (as in Arduino code line 246)
    uint8_t midi_value = value / 2;
    return midi_create_control_change(MIDI_CHANNEL, cc_number, midi_value);
}

midi_message_t midi_converter_jog_to_cc(uint8_t cc_number, uint8_t old_value, uint8_t new_value)
{
    // Jog wheels are rotary encoders with wrap-around
    // Just send the new position divided by 2 for MIDI range
    uint8_t midi_value = new_value / 2;
    return midi_create_control_change(MIDI_CHANNEL, cc_number, midi_value);
}
