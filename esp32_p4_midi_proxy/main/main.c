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

static const char *TAG = "main";

#define FIRMWARE_VERSION "0.1.0"

// Task priorities
#define USB_HOST_TASK_PRIORITY   (tskIDLE_PRIORITY + 5)
#define USB_DEVICE_TASK_PRIORITY (tskIDLE_PRIORITY + 5)

// Task stack sizes
#define USB_HOST_STACK_SIZE   4096
#define USB_DEVICE_STACK_SIZE 4096

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-P4 USB MIDI Proxy starting...");
    ESP_LOGI(TAG, "Firmware version: %s", FIRMWARE_VERSION);

    // Initialize TinyUSB
    ESP_LOGI(TAG, "Initializing TinyUSB...");

    // Initialize USB Host (OTG1)
    usb_host_init();

    // Initialize USB Device (OTG2)
    usb_device_init();

    // Initialize TinyUSB stack
    tusb_init();

    // Create USB Host task
    xTaskCreate(usb_host_task, "usb_host", USB_HOST_STACK_SIZE, NULL,
                USB_HOST_TASK_PRIORITY, NULL);
    ESP_LOGI(TAG, "USB Host task created");

    // Create USB Device task
    xTaskCreate(usb_device_task, "usb_device", USB_DEVICE_STACK_SIZE, NULL,
                USB_DEVICE_TASK_PRIORITY, NULL);
    ESP_LOGI(TAG, "USB Device task created");

    ESP_LOGI(TAG, "Initialization complete");
    ESP_LOGI(TAG, "System ready - waiting for USB connections...");
}
