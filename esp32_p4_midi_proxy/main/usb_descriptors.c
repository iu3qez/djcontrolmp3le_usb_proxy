/*
 * USB Device Descriptors for TinyUSB
 *
 * Composite device: MIDI + CDC (debug serial)
 */

#include "tusb.h"

// Device Descriptor
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,  // Composite device
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xCAFE,  // TODO: Get proper VID
    .idProduct          = 0x4001,  // TODO: Get proper PID
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*) &desc_device;
}

// Configuration Descriptor
enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_MIDI,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN)

#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82
#define EPNUM_MIDI_OUT    0x03
#define EPNUM_MIDI_IN     0x83

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

    // MIDI: Interface number, string index, EP Out & In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 5, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

// String Descriptors
char const* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 }, // 0: supported language is English (0x0409)
    "Espressif",                   // 1: Manufacturer
    "ESP32-P4 MIDI Proxy",         // 2: Product
    "123456",                      // 3: Serial (TODO: use chip ID)
    "CDC Serial",                  // 4: CDC Interface
    "MIDI",                        // 5: MIDI Interface
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        if (!(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        // Convert ASCII string into UTF-16
        for(uint8_t i=0; i<chr_count; i++) {
            _desc_str[1+i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

    return _desc_str;
}
