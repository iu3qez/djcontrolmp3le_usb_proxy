/*
 * MIDI Converter
 *
 * Converts Hercules controller state changes to MIDI messages
 */

#ifndef MIDI_CONVERTER_H
#define MIDI_CONVERTER_H

#include <stdint.h>
#include <stdbool.h>
#include "midi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// MIDI channel for all messages (1-16)
#define MIDI_CHANNEL 0  // Channel 1 (0-indexed)

/**
 * Initialize MIDI converter
 */
void midi_converter_init(void);

/**
 * Process Hercules state and generate MIDI messages
 *
 * @param current Current 38-byte state from Hercules
 * @param previous Previous 38-byte state
 * @return Number of MIDI messages generated and queued
 */
uint8_t midi_converter_process(const uint8_t *current, const uint8_t *previous);

/**
 * Convert a single button change to MIDI Note On/Off
 *
 * @param note_number MIDI note number (0-127)
 * @param is_pressed true if button is pressed, false if released
 * @return MIDI message
 */
midi_message_t midi_converter_button_to_note(uint8_t note_number, bool is_pressed);

/**
 * Convert a dial/slider value to MIDI CC
 *
 * @param cc_number MIDI CC number (0-127)
 * @param value Raw value from Hercules (0-255)
 * @return MIDI message
 */
midi_message_t midi_converter_dial_to_cc(uint8_t cc_number, uint8_t value);

/**
 * Convert a jog wheel delta to MIDI CC
 *
 * @param cc_number MIDI CC number (0-127)
 * @param old_value Previous jog position (0-255)
 * @param new_value Current jog position (0-255)
 * @return MIDI message
 */
midi_message_t midi_converter_jog_to_cc(uint8_t cc_number, uint8_t old_value, uint8_t new_value);

#ifdef __cplusplus
}
#endif

#endif // MIDI_CONVERTER_H
