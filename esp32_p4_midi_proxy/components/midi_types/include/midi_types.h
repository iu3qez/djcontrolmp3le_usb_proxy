/*
 * MIDI Types and Constants
 *
 * Defines MIDI message structures and helper functions
 */

#ifndef MIDI_TYPES_H
#define MIDI_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// MIDI message structure (3 bytes)
typedef struct {
    uint8_t status;  // Status byte (includes message type and channel)
    uint8_t data1;   // First data byte
    uint8_t data2;   // Second data byte
} midi_message_t;

// MIDI message types (status byte high nibble)
#define MIDI_NOTE_OFF           0x80
#define MIDI_NOTE_ON            0x90
#define MIDI_POLY_AFTERTOUCH    0xA0
#define MIDI_CONTROL_CHANGE     0xB0
#define MIDI_PROGRAM_CHANGE     0xC0
#define MIDI_CHANNEL_AFTERTOUCH 0xD0
#define MIDI_PITCH_BEND         0xE0
#define MIDI_SYSTEM             0xF0

// Helper macros
#define MIDI_GET_TYPE(status)    ((status) & 0xF0)
#define MIDI_GET_CHANNEL(status) ((status) & 0x0F)
#define MIDI_MAKE_STATUS(type, channel) (((type) & 0xF0) | ((channel) & 0x0F))

// Helper functions
midi_message_t midi_create_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
midi_message_t midi_create_note_off(uint8_t channel, uint8_t note, uint8_t velocity);
midi_message_t midi_create_control_change(uint8_t channel, uint8_t controller, uint8_t value);

bool midi_is_note_on(const midi_message_t *msg);
bool midi_is_note_off(const midi_message_t *msg);
bool midi_is_control_change(const midi_message_t *msg);

#ifdef __cplusplus
}
#endif

#endif // MIDI_TYPES_H
