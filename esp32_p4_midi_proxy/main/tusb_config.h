/*
 * TinyUSB Configuration for ESP32-P4 Dual USB OTG
 */

#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// Board Specific Configuration
//--------------------------------------------------------------------

// RHPort number used for host (OTG1)
#define BOARD_TUH_RHPORT      0

// RHPort max operational speed for host
#define BOARD_TUH_MAX_SPEED   OPT_MODE_FULL_SPEED

// RHPort number used for device (OTG2)
#define BOARD_TUD_RHPORT      1

// RHPort max operational speed for device
#define BOARD_TUD_MAX_SPEED   OPT_MODE_FULL_SPEED

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by compiler flags
#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU OPT_MCU_ESP32P4
#endif

#ifndef CFG_TUSB_OS
  #define CFG_TUSB_OS OPT_OS_FREERTOS
#endif

#ifndef CFG_TUSB_DEBUG
  #define CFG_TUSB_DEBUG 0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED       1
#define CFG_TUD_MAX_SPEED     BOARD_TUD_MAX_SPEED

// Enable Host stack
#define CFG_TUH_ENABLED       1
#define CFG_TUH_MAX_SPEED     BOARD_TUH_MAX_SPEED

// RHPort mode configuration (REQUIRED for dual USB)
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_HOST | OPT_MODE_FULL_SPEED)   // OTG1 as Host
#define CFG_TUSB_RHPORT1_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED) // OTG2 as Device

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
  #define CFG_TUD_ENDPOINT0_SIZE 64
#endif

// CDC
#define CFG_TUD_CDC           1
#define CFG_TUD_CDC_RX_BUFSIZE 256
#define CFG_TUD_CDC_TX_BUFSIZE 256

// MIDI
#define CFG_TUD_MIDI          1
#define CFG_TUD_MIDI_RX_BUFSIZE 64
#define CFG_TUD_MIDI_TX_BUFSIZE 64

// Disabled device classes
#define CFG_TUD_MSC           0
#define CFG_TUD_HID           0
#define CFG_TUD_AUDIO         0
#define CFG_TUD_VIDEO         0
#define CFG_TUD_VENDOR        0
#define CFG_TUD_NET           0
#define CFG_TUD_BTH           0
#define CFG_TUD_DFU           0
#define CFG_TUD_DFU_RUNTIME   0
#define CFG_TUD_ECM_RNDIS     0
#define CFG_TUD_NCM           0

//--------------------------------------------------------------------
// HOST CONFIGURATION
//--------------------------------------------------------------------

// Size of buffer to hold descriptors and other data for enumeration
#define CFG_TUH_ENUMERATION_BUFSIZE 256

// Number of hub devices
#define CFG_TUH_HUB           0

// max device support (excluding hub)
#define CFG_TUH_DEVICE_MAX    1

// Number of mass storage
#define CFG_TUH_MSC           0

// Number of HIDs
#define CFG_TUH_HID           1

// Number of CDC
#define CFG_TUH_CDC           0

// Number of MIDI
#define CFG_TUH_MIDI          0

// Number of Vendor class device
#define CFG_TUH_VENDOR        0

// HID buffer size
#define CFG_TUH_HID_EPIN_BUFSIZE  64
#define CFG_TUH_HID_EPOUT_BUFSIZE 64

#ifdef __cplusplus
}
#endif

#endif // TUSB_CONFIG_H
