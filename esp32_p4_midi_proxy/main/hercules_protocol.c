/*
 * Hercules DJControl MP3 LE Protocol Implementation
 *
 * Based on djcontrolmp3le_usb_proxy.ino mapping table
 */

#include "hercules_protocol.h"

// Control mapping table (from Arduino code lines 149-214)
const hercules_control_t hercules_controls[HERCULES_CONTROL_COUNT] = {
    // Buttons (46 total) - mapped to MIDI notes 0-45
    { "PitchReset_A",      4, 0x80, CONTROL_TYPE_BUTTON, 0 },
    { "PitchBendMinus_A",  0, 0x02, CONTROL_TYPE_BUTTON, 1 },
    { "PitchBendPlus_A",   0, 0x04, CONTROL_TYPE_BUTTON, 2 },
    { "Sync_A",            4, 0x20, CONTROL_TYPE_BUTTON, 3 },
    { "Shift_A",           0, 0x01, CONTROL_TYPE_BUTTON, 4 },
    { "Shifted_A",         3, 0x10, CONTROL_TYPE_BUTTON, 5 },
    { "N1_A",              4, 0x40, CONTROL_TYPE_BUTTON, 6 },
    { "N2_A",              0, 0x10, CONTROL_TYPE_BUTTON, 7 },
    { "N3_A",              0, 0x20, CONTROL_TYPE_BUTTON, 8 },
    { "N4_A",              0, 0x40, CONTROL_TYPE_BUTTON, 9 },
    { "N5_A",              5, 0x01, CONTROL_TYPE_BUTTON, 10 },
    { "N6_A",              5, 0x02, CONTROL_TYPE_BUTTON, 11 },
    { "N7_A",              5, 0x04, CONTROL_TYPE_BUTTON, 12 },
    { "N8_A",              5, 0x08, CONTROL_TYPE_BUTTON, 13 },
    { "RWD_A",             0, 0x08, CONTROL_TYPE_BUTTON, 14 },
    { "FWD_A",             0, 0x80, CONTROL_TYPE_BUTTON, 15 },
    { "CUE_A",             1, 0x02, CONTROL_TYPE_BUTTON, 16 },
    { "Play_A",            1, 0x04, CONTROL_TYPE_BUTTON, 17 },
    { "Listen_A",          1, 0x01, CONTROL_TYPE_BUTTON, 18 },
    { "Load_A",            1, 0x08, CONTROL_TYPE_BUTTON, 19 },
    { "PitchReset_B",      4, 0x02, CONTROL_TYPE_BUTTON, 20 },
    { "PitchBendMinus_B",  3, 0x02, CONTROL_TYPE_BUTTON, 21 },
    { "PitchBendPlus_B",   3, 0x04, CONTROL_TYPE_BUTTON, 22 },
    { "Sync_B",            4, 0x08, CONTROL_TYPE_BUTTON, 23 },
    { "Shift_B",           3, 0x01, CONTROL_TYPE_BUTTON, 24 },
    { "Shifted_B",         3, 0x20, CONTROL_TYPE_BUTTON, 25 },
    { "N1_B",              4, 0x04, CONTROL_TYPE_BUTTON, 26 },
    { "N2_B",              2, 0x10, CONTROL_TYPE_BUTTON, 27 },
    { "N3_B",              2, 0x20, CONTROL_TYPE_BUTTON, 28 },
    { "N4_B",              2, 0x40, CONTROL_TYPE_BUTTON, 29 },
    { "N5_B",              5, 0x10, CONTROL_TYPE_BUTTON, 30 },
    { "N6_B",              5, 0x20, CONTROL_TYPE_BUTTON, 31 },
    { "N7_B",              5, 0x40, CONTROL_TYPE_BUTTON, 32 },
    { "N8_B",              5, 0x80, CONTROL_TYPE_BUTTON, 33 },
    { "RWD_B",             3, 0x08, CONTROL_TYPE_BUTTON, 34 },
    { "FWD_B",             2, 0x80, CONTROL_TYPE_BUTTON, 35 },
    { "CUE_B",             2, 0x02, CONTROL_TYPE_BUTTON, 36 },
    { "Play_B",            2, 0x04, CONTROL_TYPE_BUTTON, 37 },
    { "Listen_B",          2, 0x01, CONTROL_TYPE_BUTTON, 38 },
    { "Load_B",            2, 0x08, CONTROL_TYPE_BUTTON, 39 },
    { "Vinyl",             4, 0x10, CONTROL_TYPE_BUTTON, 40 },
    { "Magic",             4, 0x01, CONTROL_TYPE_BUTTON, 41 },
    { "Up",                1, 0x10, CONTROL_TYPE_BUTTON, 42 },
    { "Down",              1, 0x80, CONTROL_TYPE_BUTTON, 43 },
    { "Folders",           1, 0x20, CONTROL_TYPE_BUTTON, 44 },
    { "Files",             1, 0x40, CONTROL_TYPE_BUTTON, 45 },

    // Dials and sliders (9 total) - mapped to CC 46-54
    { "Treble_A",          7, 0xff, CONTROL_TYPE_DIAL, 46 },
    { "Medium_A",          8, 0xff, CONTROL_TYPE_DIAL, 47 },
    { "Bass_A",            9, 0xff, CONTROL_TYPE_DIAL, 48 },
    { "Vol_A",             6, 0xff, CONTROL_TYPE_DIAL, 49 },
    { "Treble_B",         12, 0xff, CONTROL_TYPE_DIAL, 50 },
    { "Medium_B",         13, 0xff, CONTROL_TYPE_DIAL, 51 },
    { "Bass_B",           14, 0xff, CONTROL_TYPE_DIAL, 52 },
    { "Vol_B",            11, 0xff, CONTROL_TYPE_DIAL, 53 },
    { "XFader",           10, 0xff, CONTROL_TYPE_DIAL, 54 },

    // Jog wheels (4 total) - mapped to CC 55-58
    { "Jog_A",            15, 0xff, CONTROL_TYPE_JOG, 55 },
    { "Pitch_A",          17, 0xff, CONTROL_TYPE_JOG, 56 },
    { "Jog_B",            16, 0xff, CONTROL_TYPE_JOG, 57 },
    { "Pitch_B",          18, 0xff, CONTROL_TYPE_JOG, 58 }
};

const char* hercules_get_control_name(uint8_t index)
{
    if (index >= HERCULES_CONTROL_COUNT) {
        return "UNKNOWN";
    }
    return hercules_controls[index].name;
}

control_type_t hercules_get_control_type(uint8_t index)
{
    if (index >= HERCULES_CONTROL_COUNT) {
        return CONTROL_TYPE_BUTTON;
    }
    return hercules_controls[index].control_type;
}

uint8_t hercules_get_midi_mapping(uint8_t index)
{
    if (index >= HERCULES_CONTROL_COUNT) {
        return 0;
    }
    return hercules_controls[index].midi_note_or_cc;
}
