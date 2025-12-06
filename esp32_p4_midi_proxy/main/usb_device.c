/*
 * USB Device Implementation (OTG2)
 *
 * Handles MIDI and CDC device functionality
 */

#include "usb_device.h"
#include "tusb.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "usb_device";

// CDC line coding
static cdc_line_coding_t cdc_line_coding = {
    .bit_rate = 115200,
    .stop_bits = 0,
    .parity = 0,
    .data_bits = 8
};

void usb_device_init(void)
{
    ESP_LOGI(TAG, "Initializing USB Device (OTG2)...");

    // TinyUSB device stack will be initialized by the framework
    // This function can be used for additional device-specific setup

    ESP_LOGI(TAG, "USB Device initialized");
}

void usb_device_task(void *pvParameters)
{
    ESP_LOGI(TAG, "USB Device task started");

    while (1) {
        // TinyUSB device task
        tud_task();

        // Small delay to prevent watchdog triggers
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// MIDI functions
bool midi_send(const midi_message_t *msg)
{
    if (!tud_midi_mounted()) {
        return false;
    }

    // MIDI USB packet: cable number (0) + message
    uint8_t packet[4] = {
        0x00,           // Cable number and CIN (will be set by write)
        msg->status,
        msg->data1,
        msg->data2
    };

    return tud_midi_stream_write(0, &packet[1], 3) == 3;
}

// CDC functions
void cdc_printf(const char *format, ...)
{
    if (!tud_cdc_connected()) {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        tud_cdc_write(buffer, len);
        tud_cdc_write_flush();
    }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "USB Device mounted");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "USB Device unmounted");
}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    ESP_LOGI(TAG, "USB Device suspended");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGI(TAG, "USB Device resumed");
}

// Invoked when CDC interface received data
void tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;

    // Read received data
    uint8_t buf[64];
    uint32_t count = tud_cdc_read(buf, sizeof(buf));

    if (count > 0) {
        // Process through CDC console
        extern void cdc_console_process(const uint8_t *data, uint16_t len);
        cdc_console_process(buf, count);
    }
}

// Invoked when CDC line state changed
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    if (dtr) {
        ESP_LOGI(TAG, "CDC terminal connected");

        // Send welcome message
        extern void cdc_console_print_help(void);
        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay for terminal to be ready
        cdc_console_print_help();
        cdc_printf("> ");
    } else {
        ESP_LOGI(TAG, "CDC terminal disconnected");
    }
}

// Invoked when CDC line coding is changed
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    (void) itf;
    memcpy(&cdc_line_coding, p_line_coding, sizeof(cdc_line_coding_t));
    ESP_LOGI(TAG, "CDC line coding: %lu baud, %u stop, %u parity, %u bits",
             cdc_line_coding.bit_rate, cdc_line_coding.stop_bits,
             cdc_line_coding.parity, cdc_line_coding.data_bits);
}

// Invoked when MIDI interface received data
void tud_midi_rx_cb(uint8_t itf)
{
    (void) itf;

    uint8_t packet[4];
    while (tud_midi_stream_read(packet, 4)) {
        // MIDI received (currently unused - device is TX only)
        ESP_LOGD(TAG, "MIDI RX: %02x %02x %02x", packet[1], packet[2], packet[3]);
    }
}
