/*
 * System Configuration
 *
 * Defines buffer sizes, queue depths, and other system constants
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// USB Buffer Sizes
//--------------------------------------------------------------------+

// USB Host RX buffer (for Hercules controller reports)
#define USB_HOST_RX_BUFFER_SIZE     256

// USB Device TX buffer (for MIDI messages to PC)
#define USB_DEVICE_TX_BUFFER_SIZE   256

// Hercules controller report size (from Arduino code)
#define HERCULES_REPORT_SIZE        38

//--------------------------------------------------------------------+
// FreeRTOS Queue Configuration
//--------------------------------------------------------------------+

// MIDI message queue depth (between converter and USB TX)
#define MIDI_QUEUE_DEPTH            16

// Maximum number of hooks in the system
#define MAX_HOOKS                   16

// Maximum LED mappings
#define MAX_LED_MAPPINGS            32

//--------------------------------------------------------------------+
// Task Configuration
//--------------------------------------------------------------------+

// Task stack sizes (in bytes)
#define USB_HOST_TASK_STACK         4096
#define USB_DEVICE_TASK_STACK       4096
#define MIDI_CONVERTER_TASK_STACK   3072
#define LED_CONTROL_TASK_STACK      3072

// Task priorities (higher number = higher priority)
#define USB_HOST_TASK_PRIORITY      (configMAX_PRIORITIES - 2)
#define USB_DEVICE_TASK_PRIORITY    (configMAX_PRIORITIES - 2)
#define MIDI_CONVERTER_TASK_PRIORITY (configMAX_PRIORITIES - 3)
#define LED_CONTROL_TASK_PRIORITY    (configMAX_PRIORITIES - 4)

//--------------------------------------------------------------------+
// Timing Configuration
//--------------------------------------------------------------------+

// USB Host polling interval (ms)
#define USB_HOST_POLL_INTERVAL_MS   1

// TCI polling interval (ms) - for Phase 4
#define TCI_POLL_INTERVAL_MS        100

// LED update rate limit (ms)
#define LED_UPDATE_MIN_INTERVAL_MS  10

//--------------------------------------------------------------------+
// Debug Configuration
//--------------------------------------------------------------------+

// Enable verbose logging
#ifndef DEBUG_VERBOSE
  #define DEBUG_VERBOSE             0
#endif

// Enable MIDI message logging
#ifndef DEBUG_MIDI_LOG
  #define DEBUG_MIDI_LOG            0
#endif

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
