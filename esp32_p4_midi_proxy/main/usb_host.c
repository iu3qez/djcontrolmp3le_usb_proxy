/*
 * USB Host Implementation (OTG1)
 *
 * Handles connection to Hercules DJControl MP3 LE controller
 */

#include "usb_host.h"
#include "tusb.h"
#include "esp_log.h"

static const char *TAG = "usb_host";

// Hercules DJControl MP3 LE identifiers
#define HERCULES_VID  0x06f8
#define HERCULES_PID  0xb105

// Static state
static bool hercules_mounted = false;
static uint8_t hercules_dev_addr = 0;
static uint8_t hercules_instance = 0;

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
        ESP_LOGI(TAG, "Hercules controller detected!");

        // Request to receive report
        if (!tuh_hid_receive_report(dev_addr, instance)) {
            ESP_LOGE(TAG, "Failed to request HID report");
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
        // Process Hercules report (will be implemented in Phase 2)
        ESP_LOGD(TAG, "Hercules report received, length=%d", len);

        // TODO: Parse report and convert to MIDI
        // For now, just log first few bytes
        if (len > 0) {
            ESP_LOGD(TAG, "Data: %02x %02x %02x %02x...",
                     report[0],
                     len > 1 ? report[1] : 0,
                     len > 2 ? report[2] : 0,
                     len > 3 ? report[3] : 0);
        }

        // Request next report
        tuh_hid_receive_report(dev_addr, instance);
    }
}
