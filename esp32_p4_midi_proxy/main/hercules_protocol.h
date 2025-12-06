/*
 * Hercules DJControl MP3 LE Protocol
 *
 * Control mapping and protocol definitions
 */

#ifndef HERCULES_PROTOCOL_H
#define HERCULES_PROTOCOL_H

#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Control types
typedef enum {
    CONTROL_TYPE_BUTTON = 0,    // Digital button (0 or 1)
    CONTROL_TYPE_DIAL = 1,      // Analog dial/slider (0-255)
    CONTROL_TYPE_JOG = 2        // Rotary encoder with wrap-around (0-255)
} control_type_t;

// Control mapping entry
typedef struct {
    const char *name;           // Control name
    uint8_t byte_offset;        // Byte offset in 38-byte report
    uint8_t byte_mask;          // Bit mask
    control_type_t control_type; // Type of control
    uint8_t midi_note_or_cc;    // MIDI note (buttons) or CC number (dials/jogs)
} hercules_control_t;

// Total number of controls
#define HERCULES_CONTROL_COUNT 59

// Get the control mapping table
extern const hercules_control_t hercules_controls[HERCULES_CONTROL_COUNT];

// Helper functions
const char* hercules_get_control_name(uint8_t index);
control_type_t hercules_get_control_type(uint8_t index);
uint8_t hercules_get_midi_mapping(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif // HERCULES_PROTOCOL_H
