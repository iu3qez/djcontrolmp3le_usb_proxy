# ESP32-P4 USB MIDI Proxy - Product Requirements Document

**Date:** 2025-11-17
**Status:** Design Phase
**Target Platform:** ESP32-P4 (Dual USB OTG)

## Overview

Reimplementation of the Hercules DJControl MP3 LE USB proxy using a single ESP32-P4 microcontroller. The ESP32-P4's dual USB OTG ports eliminate the need for multiple microcontrollers and serial communication, providing a simpler, lower-latency solution.

### Goals

- Convert proprietary Hercules USB protocol to class-compliant MIDI
- Minimize latency for real-time DJ controller use
- Provide extensible hook system for MIDI stream processing
- Use C pure with ESP-IDF for deterministic performance
- Single device solution leveraging ESP32-P4 dual USB capabilities

### Non-Goals

- Multi-language support (MicroPython/Rust considered but rejected for latency reasons)
- Dynamic configuration UI (compile-time + NVS-based config only)
- Support for other DJ controllers (focus on Hercules MP3 LE initially)

## Hardware Architecture

### Core Components

**ESP32-P4 Configuration:**
- USB Port 1: **Host mode** → Hercules DJControl MP3 LE
- USB Port 2: **Device mode** → PC (MIDI class-compliant + CDC debug)
- Dual-core RISC-V @ 400MHz
- Power: Via USB from PC side

### Debug & Status Indicators

**LED Configuration:**
- LED1: System status (ready/error/activity)
- LED2: MIDI activity indicator

**Debug Interface:**
- USB CDC serial on Device port (composite with MIDI)
- Optional external UART for secondary debug channel

### GPIO Expansion (Thetis SDR Integration)

**External LED Control:**
- Configurable GPIO pins (up to 16 mappable outputs)
- MIDI event → GPIO mapping via hook system
- Support for active-high/active-low configuration
- Optional PWM for LED brightness (MIDI velocity → duty cycle)

**Use Case:**
- Thetis SDR radio state visualization
- MIDI-controlled relay/indicator board
- External hardware synchronization

### Power Management

**Primary Plan:**
- Power from PC via USB Device port
- ESP32-P4 provides power to Hercules via USB Host port

**Fallback:**
- External 5V supply if USB Host power insufficient for Hercules

## Software Architecture

### Technology Stack

- **Framework:** ESP-IDF (C pure)
- **USB Stack:** TinyUSB (host + device)
- **RTOS:** FreeRTOS (included in ESP-IDF)
- **Build System:** CMake + ESP-IDF toolchain

### MIDI Data Pipeline

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐     ┌─────────────┐
│ USB Host RX │ --> │  Hercules    │ --> │ Hook Chain  │ --> │ USB Device  │
│  (Hercules) │     │  → MIDI      │     │  Pipeline   │     │  TX (PC)    │
└─────────────┘     └──────────────┘     └─────────────┘     └─────────────┘
```

**Buffer Strategy:**
- Static allocation only (no dynamic memory at runtime)
- 256 bytes RX/TX buffers for USB endpoints
- Ring buffers for inter-task communication via FreeRTOS queues

### Component Structure

```
/main
  ├── main.c                    // Entry point, FreeRTOS task creation
  ├── usb_host_hercules.c/h     // USB Host handling, Hercules parsing
  ├── usb_device_midi.c/h       // USB Device MIDI class + CDC debug
  ├── midi_converter.c/h        // Hercules protocol → MIDI mapping
  ├── hook_system.c/h           // Hook chain + built-in hooks
  ├── gpio_controller.c/h       // GPIO output management for LED control
  └── config.c/h                // NVS configuration management

/components
  └── midi_types/               // Common MIDI definitions and types
      ├── midi_types.h
      └── CMakeLists.txt
```

### Task Architecture

**FreeRTOS Tasks:**

1. **USB Host Task** (Priority: High)
   - Reads data from Hercules controller
   - Calls MIDI converter
   - Pushes to hook chain queue

2. **Hook Processing Task** (Priority: Medium)
   - Receives MIDI messages from queue
   - Executes hook chain
   - Outputs to USB Device queue

3. **USB Device Task** (Priority: High)
   - Receives MIDI from hook chain
   - Transmits to PC via USB MIDI

**Inter-task Communication:**
- FreeRTOS queues with static allocation
- No direct shared memory (thread-safe design)

## Hook System Design

### Architecture

**Hook Chain:**
```c
typedef bool (*midi_hook_t)(midi_message_t* msg);

typedef struct {
    midi_hook_t hook_fn;
    bool enabled;
    const char* name;
} hook_entry_t;

#define MAX_HOOKS 16
extern hook_entry_t g_hook_chain[MAX_HOOKS];
```

**Hook Behavior:**
- Each hook receives pointer to MIDI message
- Can modify message in-place
- Return `false` to discard message
- Return `true` to continue chain

### Built-in Hooks (Compile-time Optional)

1. **Velocity Adjustment** - Normalize/amplify note velocity
2. **Note Remapping** - Transpose notes, remap keys
3. **CC Filter** - Block specific control changes
4. **MIDI Logger** - Debug stream to serial/CDC
5. **GPIO Output** - Drive external LEDs/signals based on MIDI events

### Primary Use Case: Thetis SDR Integration

**Objective:** Control external LEDs in response to specific MIDI events that correspond to Thetis SDR radio states.

**Implementation:**
- GPIO hook monitors specific MIDI messages (CC, notes, or custom mappings)
- Maps MIDI events to GPIO pin states (LED on/off)
- Configuration via NVS: `midi_note_X → GPIO_pin_Y`
- Example: MIDI Note 60 (C4) → GPIO 12 HIGH (LED ON)

**Hardware Extension:**
- ESP32-P4 GPIO pins connected to LED drivers
- Configurable active-high/active-low per pin
- Support for PWM on LEDs (brightness control via MIDI velocity)

**Configuration:**
- Compile-time: `#ifdef CONFIG_HOOK_VELOCITY_ADJUST`
- Runtime: Enable/disable via NVS flags
- Debug commands: `hook enable velocity_fix`

### Hook Registration

**Static Registration (Compile-time):**
```c
void register_builtin_hooks(void) {
    #ifdef CONFIG_HOOK_VELOCITY_ADJUST
    hook_register("velocity_adjust", velocity_adjust_hook);
    #endif
    // ... more hooks
}
```

**Runtime Enable/Disable:**
- Stored in NVS (Non-Volatile Storage)
- Configurable via CDC serial commands
- Survives reboots

## Hercules Protocol Conversion

### Source Material

- Reuse mapping logic from original Arduino/Teensy project
- Reference: [djcontrol project](https://github.com/foomatic/djcontrol) for protocol details
- Expected to be mostly lookup table based (direct mapping)

### Implementation Approach

- Port existing Arduino code to C for ESP-IDF
- Likely static lookup tables for buttons/knobs → MIDI CC/Notes
- Minimal processing overhead (simple translations)

## Configuration & Debug

### NVS Configuration Schema

```c
typedef struct {
    bool hooks_enabled[MAX_HOOKS];
    uint8_t led_brightness;
    bool debug_logging;
    // Hook-specific configs
    int8_t velocity_offset;
    uint8_t note_transpose;
} config_t;
```

**Access:**
- Read on boot
- Modified via serial commands
- Committed to NVS on command

### Debug Commands (CDC Serial)

- `status` - Show system state, enabled hooks
- `hook list` - List available hooks
- `hook enable <name>` - Enable hook
- `hook disable <name>` - Disable hook
- `config save` - Persist to NVS
- `midi dump` - Enable MIDI stream logging

## Performance Requirements

### Latency Targets

- **End-to-end latency:** < 5ms (Hercules USB → PC MIDI out)
- **Hook processing:** < 1ms per message
- **Buffer depth:** Minimal (256 bytes = ~85 MIDI messages)

### Throughput

- **MIDI bandwidth:** Standard MIDI @ 31.25 kbps equivalent
- **USB bandwidth:** Full Speed (12 Mbps) sufficient
- **Burst handling:** Support simultaneous button presses (up to 16 concurrent events)

### Determinism

- No dynamic memory allocation in MIDI path
- Fixed-priority FreeRTOS scheduling
- Interrupt-driven USB transfers
- No garbage collection (C pure, no scripts)

## Testing Strategy

### Unit Testing

- MIDI conversion lookup tables (Hercules → MIDI)
- Hook system (registration, enable/disable, chain execution)
- Buffer management (no overflow under burst)

### Integration Testing

- USB Host enumeration with Hercules controller
- USB Device enumeration as MIDI class on PC
- End-to-end MIDI flow with real controller
- Hook chain with multiple enabled hooks

### Performance Testing

- Latency measurement (oscilloscope or software timing)
- Sustained load (rapid button mashing)
- Edge cases (simultaneous multi-control input)

## Development Phases

### Phase 1: Core Infrastructure
- ESP-IDF project setup
- TinyUSB integration (host + device)
- Basic USB enumeration (both sides)
- Static buffer allocation

### Phase 2: MIDI Pipeline
- Port Hercules protocol conversion
- MIDI message types and definitions
- Basic pipeline: USB RX → Convert → USB TX
- No hooks yet (straight passthrough)

### Phase 3: Hook System
- Hook registration framework
- Built-in hook implementations
- NVS configuration support
- Serial debug commands

### Phase 4: Testing & Refinement
- Hardware testing with real Hercules controller
- Latency optimization
- Documentation
- Example hook implementations

## Open Questions

1. **ESP32-P4 USB Host/Device simultaneous mode**: Verify TinyUSB supports both modes concurrently on P4
2. **Power budget**: Confirm ESP32-P4 can provide sufficient USB Host current for Hercules
3. **Hercules protocol details**: May need additional reverse-engineering beyond djcontrol docs

## References

- [Original Teensy implementation](https://github.com/pr8x/djcontrolmp3le_usb_proxy)
- [djcontrol protocol research](https://github.com/foomatic/djcontrol)
- [ESP32-P4 datasheet](https://www.espressif.com/en/products/socs/esp32-p4)
- [TinyUSB documentation](https://docs.tinyusb.org/)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)

## Success Criteria

- [ ] Hercules controller recognized and functional on modern OS without drivers
- [ ] MIDI mapping matches or exceeds original Teensy implementation
- [ ] End-to-end latency < 5ms
- [ ] At least 3 useful built-in hooks implemented
- [ ] Debug interface functional for troubleshooting
- [ ] Clean, maintainable C codebase with documentation

## Future Enhancements (Out of Scope for V1)

- Support for additional DJ controllers
- Web-based configuration interface
- Wireless mode (ESP32-P4 with external WiFi module)
- Advanced hook scripting (Lua/WASM bytecode execution)
- MIDI learn mode for custom mappings
