# ESP32-P4 USB MIDI Proxy

Firmware for ESP32-P4 that converts Hercules DJControl MP3 LE USB controller to a class-compliant MIDI device with LED feedback control.

## Features

- **USB Host Mode**: Connects to Hercules controller (VID: 0x06f8, PID: 0xb105)
- **USB Device Mode**: Appears as MIDI device on PC
- **MIDI Conversion**: All 46 buttons, 9 dials/sliders, 4 jog wheels
- **LED Control**: Control Hercules LEDs based on radio state (via ESP32-C6 TCI client)
- **CDC Debug Serial**: USB serial for debugging and configuration
- **Hook System**: Extensible MIDI processing pipeline

## Project Structure

```
esp32_p4_midi_proxy/
├── main/                      # Main application code
│   ├── main.c                 # Entry point
│   └── CMakeLists.txt
├── components/
│   ├── midi_types/            # MIDI message types and helpers
│   └── esp_hosted/            # ESP-Hosted for SDIO (placeholder)
├── CMakeLists.txt             # Project build configuration
└── sdkconfig.defaults         # Default ESP-IDF configuration
```

## Building

```bash
cd esp32_p4_midi_proxy
idf.py set-target esp32p4
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Development Status

See [Implementation Tasks](../docs/plans/2025-11-17-implementation-tasks.md) for detailed progress.

**Current Phase**: Phase 2.5 - FreeRTOS Task Pipeline

**Completed:**
- ✅ Phase 1 - ESP32-P4 Core Infrastructure
- ✅ Phase 2.1 - MIDI Types Component
- ✅ Phase 2.2 - USB Host Hercules Driver
- ✅ Phase 2.3 - Hercules Protocol Conversion
- ✅ Phase 2.4 - USB Device MIDI Class
- ✅ Phase 2.5 - FreeRTOS Task Pipeline

**Architecture:** See [ARCHITECTURE.md](ARCHITECTURE.md) for system design details.

## Reference

Original Arduino code is in `../old/djcontrolmp3le_usb_proxy.ino`
