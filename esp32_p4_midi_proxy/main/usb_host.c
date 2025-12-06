/*
 * USB Host Implementation (OTG1)
 *
 * Handles connection to Hercules DJControl MP3 LE controller
 */

#include "usb_host.h"
#include "tusb.h"
#include "esp_log.h"
#include "buffers.h"
#include "midi_converter.h"
#include "config.h"
#include <string.h>

static const char *TAG = "usb_host";

// Hercules DJControl MP3 LE identifiers
#define HERCULES_VID  0x06f8
#define HERCULES_PID  0xb105

// Hercules init sequence (from Arduino code lines 30-45)
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} hercules_ctrl_transfer_t;

static const hercules_ctrl_transfer_t hercules_init_sequence[] = {
    { 0xc0, 0x2c, 0x0000, 0x0000, 2 },  // => 4040
    { 0xc0, 0x29, 0x0300, 0x0000, 2 },  // => 0c0c
    { 0xc0, 0x29, 0x0400, 0x0000, 2 },  // => f2f2
    { 0xc0, 0x29, 0x0500, 0x0000, 2 },  // => eded
    { 0xc0, 0x29, 0x0600, 0x0000, 2 },  // => 7373
    { 0xc0, 0x2c, 0x0000, 0x0000, 2 },  // => 4040
    { 0xc0, 0x2c, 0x0000, 0x0000, 2 },  // => 4040
    { 0xc0, 0x29, 0x0300, 0x0000, 2 },  // => 0c0c
    { 0xc0, 0x29, 0x0400, 0x0000, 2 },  // => f2f2
    { 0xc0, 0x29, 0x0500, 0x0000, 2 },  // => eded
    { 0xc0, 0x29, 0x0600, 0x0000, 2 },  // => 7373
    { 0xc0, 0x29, 0x0200, 0x0000, 2 },  // => 0000
    { 0x02, 0x01, 0x0000, 0x0082, 0 },  // CLEAR_FEATURE
    { 0x40, 0x27, 0x0000, 0x0000, 0 }   // Final command
};

#define HERCULES_INIT_SEQUENCE_COUNT (sizeof(hercules_init_sequence) / sizeof(hercules_ctrl_transfer_t))

// Static state
static bool hercules_mounted = false;
static uint8_t hercules_dev_addr = 0;
static uint8_t hercules_instance = 0;
static uint8_t hercules_init_step = 0;

void usb_host_init(void)
{
    ESP_LOGI(TAG, "Initializing USB Host (OTG1)...");

    // TinyUSB host stack will be initialized by the framework
    // This function can be used for additional host-specific setup

    ESP_LOGI(TAG, "USB Host initialized");
    ESP_LOGI(TAG, "Waiting for Hercules controller (VID:0x%04x PID:0x%04x)...",
             HERCULES_VID, HERCULES_PID);
}

void usb_host_task(void *pvParameters)
{
    ESP_LOGI(TAG, "USB Host task started");

    while (1) {
        // TinyUSB host task
        tuh_task();

        // Small delay to prevent watchdog triggers
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

bool is_hercules_mounted(void)
{
    return hercules_mounted;
}

//--------------------------------------------------------------------+
// TinyUSB Host Callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted (configured)
void tuh_mount_cb(uint8_t dev_addr)
{
    ESP_LOGI(TAG, "Device attached, address = %d", dev_addr);

    // Device descriptor will be queried by TinyUSB internally
    // We'll get VID/PID in the HID mount callback
}

// Invoked when device is unmounted
void tuh_umount_cb(uint8_t dev_addr)
{
    ESP_LOGI(TAG, "Device detached, address = %d", dev_addr);

    if (dev_addr == hercules_dev_addr) {
        hercules_mounted = false;
        hercules_dev_addr = 0;
        hercules_instance = 0;
        ESP_LOGW(TAG, "Hercules controller disconnected");
    }
}

// Invoked when HID device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    (void) desc_report;
    (void) desc_len;

    // Get VID/PID
    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    ESP_LOGI(TAG, "HID device mounted: VID=%04x PID=%04x Instance=%d", vid, pid, instance);

    // Check if this is the Hercules controller
    if (vid == HERCULES_VID && pid == HERCULES_PID) {
        hercules_mounted = true;
        hercules_dev_addr = dev_addr;
        hercules_instance = instance;
        hercules_init_step = 0;

        ESP_LOGI(TAG, "Hercules controller detected!");
        ESP_LOGI(TAG, "Sending init sequence (%d transfers)...", HERCULES_INIT_SEQUENCE_COUNT);

        // TODO: Send init sequence via control transfers
        // For now, TinyUSB doesn't provide easy control transfer API from host callbacks
        // This will need to be implemented in Phase 2.2 completion
        // Workaround: Skip init for now, Hercules might work without it

        ESP_LOGW(TAG, "Init sequence not yet implemented - trying without init");

        // Request to receive report
        if (!tuh_hid_receive_report(dev_addr, instance)) {
            ESP_LOGE(TAG, "Failed to request HID report");
        } else {
            ESP_LOGI(TAG, "Waiting for Hercules reports...");
        }
    }
}

// Invoked when HID device is unmounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    ESP_LOGI(TAG, "HID device unmounted: Address=%d Instance=%d", dev_addr, instance);

    if (dev_addr == hercules_dev_addr && instance == hercules_instance) {
        hercules_mounted = false;
        hercules_dev_addr = 0;
        hercules_instance = 0;
        ESP_LOGW(TAG, "Hercules controller disconnected");
    }
}

// Invoked when received report from HID device
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
    if (dev_addr == hercules_dev_addr && instance == hercules_instance) {
        // Hercules reports are 38 bytes
        if (len == HERCULES_REPORT_SIZE) {
            // Copy to current state buffer
            memcpy(g_hercules_state.current, report, HERCULES_REPORT_SIZE);

            // Check if state has changed
            if (hercules_state_has_changed()) {
                // Convert changes to MIDI messages
                uint8_t midi_count = midi_converter_process(
                    g_hercules_state.current,
                    g_hercules_state.previous
                );

                if (midi_count > 0) {
                    ESP_LOGD(TAG, "Generated %d MIDI messages", midi_count);
                }

                // Update previous state
                hercules_state_update();
            }
        } else {
            ESP_LOGW(TAG, "Unexpected report length: %d (expected %d)", len, HERCULES_REPORT_SIZE);
        }

        // Request next report
        tuh_hid_receive_report(dev_addr, instance);
    }
}
