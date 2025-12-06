/*
 * ESP32-P4 USB MIDI Proxy
 *
 * Main entry point for the firmware
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "tusb.h"
#include "usb_host.h"
#include "usb_device.h"
#include "buffers.h"
#include "cdc_console.h"
#include "config.h"

static const char *TAG = "main";

#define FIRMWARE_VERSION "0.1.0"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-P4 USB MIDI Proxy starting...");
    ESP_LOGI(TAG, "Firmware version: %s", FIRMWARE_VERSION);

    // Initialize static buffers
    ESP_LOGI(TAG, "Initializing buffers...");
    buffers_init();

    // Initialize MIDI converter
    ESP_LOGI(TAG, "Initializing MIDI converter...");
    extern void midi_converter_init(void);
    midi_converter_init();

    // Initialize CDC console
    ESP_LOGI(TAG, "Initializing CDC console...");
    cdc_console_init();

    // Initialize TinyUSB
    ESP_LOGI(TAG, "Initializing TinyUSB...");

    // Initialize USB Host (OTG1)
    usb_host_init();

    // Initialize USB Device (OTG2)
    usb_device_init();

    // Initialize TinyUSB stack
    tusb_init();

    // Create USB Host task
    xTaskCreate(usb_host_task, "usb_host", USB_HOST_TASK_STACK, NULL,
                USB_HOST_TASK_PRIORITY, NULL);
    ESP_LOGI(TAG, "USB Host task created");

    // Create USB Device task
    xTaskCreate(usb_device_task, "usb_device", USB_DEVICE_TASK_STACK, NULL,
                USB_DEVICE_TASK_PRIORITY, NULL);
    ESP_LOGI(TAG, "USB Device task created");

    // Create MIDI TX task
    extern void midi_tx_task(void *pvParameters);
    xTaskCreate(midi_tx_task, "midi_tx", MIDI_CONVERTER_TASK_STACK, NULL,
                MIDI_CONVERTER_TASK_PRIORITY, NULL);
    ESP_LOGI(TAG, "MIDI TX task created");

    ESP_LOGI(TAG, "Initialization complete");
    ESP_LOGI(TAG, "System ready - waiting for USB connections...");
    ESP_LOGI(TAG, "Connect via USB CDC serial and type 'help' for commands");
}
