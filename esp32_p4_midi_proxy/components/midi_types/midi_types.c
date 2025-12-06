/*
 * MIDI Types Implementation
 */

#include "midi_types.h"

midi_message_t midi_create_note_on(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi_message_t msg = {
        .status = MIDI_MAKE_STATUS(MIDI_NOTE_ON, channel),
        .data1 = note & 0x7F,
        .data2 = velocity & 0x7F
    };
    return msg;
}

midi_message_t midi_create_note_off(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi_message_t msg = {
        .status = MIDI_MAKE_STATUS(MIDI_NOTE_OFF, channel),
        .data1 = note & 0x7F,
        .data2 = velocity & 0x7F
    };
    return msg;
}

midi_message_t midi_create_control_change(uint8_t channel, uint8_t controller, uint8_t value)
{
    midi_message_t msg = {
        .status = MIDI_MAKE_STATUS(MIDI_CONTROL_CHANGE, channel),
        .data1 = controller & 0x7F,
        .data2 = value & 0x7F
    };
    return msg;
}

bool midi_is_note_on(const midi_message_t *msg)
{
    return MIDI_GET_TYPE(msg->status) == MIDI_NOTE_ON && msg->data2 > 0;
}

bool midi_is_note_off(const midi_message_t *msg)
{
    uint8_t type = MIDI_GET_TYPE(msg->status);
    return type == MIDI_NOTE_OFF || (type == MIDI_NOTE_ON && msg->data2 == 0);
}

bool midi_is_control_change(const midi_message_t *msg)
{
    return MIDI_GET_TYPE(msg->status) == MIDI_CONTROL_CHANGE;
}
