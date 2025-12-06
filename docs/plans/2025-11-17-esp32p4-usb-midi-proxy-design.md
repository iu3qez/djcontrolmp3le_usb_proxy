# ESP32-P4 USB MIDI Proxy - Product Requirements Document

**Date:** 2025-11-17
**Status:** Design Phase
**Target Platform:** GUITION ESP32-P4 + ESP32-C6 Development Board

## Overview

Reimplementation of the Hercules DJControl MP3 LE USB proxy with **bidirectional communication** for Thetis SDR integration. The GUITION board combines ESP32-P4 (dual USB OTG) with ESP32-C6 (WiFi co-processor) to enable:

1. **DJ Controller → PC**: Convert proprietary Hercules protocol to class-compliant MIDI
2. **Thetis SDR → Controller LEDs**: Read radio state via TCI protocol and control controller LEDs in real-time

### Goals

- Convert proprietary Hercules USB protocol to class-compliant MIDI
- **Bidirectional LED control**: Drive Hercules controller LEDs based on Thetis SDR state (via TCI over WiFi)
- Minimize latency for real-time DJ controller and radio control use
- Provide extensible hook system for MIDI stream processing
- Use C pure with ESP-IDF for deterministic performance
- Leverage ESP32-C6 as WiFi co-processor via esp_hosted (SDIO)

### Non-Goals

- Multi-language support (MicroPython/Rust considered but rejected for latency reasons)
- Dynamic configuration UI (compile-time + NVS-based config only)
- Support for other DJ controllers (focus on Hercules MP3 LE initially)

## Hardware Architecture

### Core Components

**GUITION Board Architecture:**

**ESP32-P4 (Primary Processor):**
- USB Port 1: **Host mode** → Hercules DJControl MP3 LE
- USB Port 2: **Device mode** → PC (MIDI class-compliant + CDC debug)
- Dual-core RISC-V @ 400MHz
- SDIO interface → ESP32-C6
- Power: Via USB from PC side

**ESP32-C6 (WiFi Co-Processor):**
- WiFi 6 (2.4 GHz) + BLE 5
- SDIO interface → ESP32-P4
- Runs **esp_hosted** firmware
- TCI client for Thetis SDR communication
- Power: Shared with P4 from board regulator

**Communication Stack:**
```
┌─────────────┐          ┌──────────────┐          ┌─────────────┐
│  Hercules   │◄──USB───►│  ESP32-P4    │◄──SDIO──►│  ESP32-C6   │
│  Controller │  Host    │              │          │   (WiFi)    │
└─────────────┘          └──────────────┘          └─────────────┘
     ▲ LED                     │ USB                      │
     │ HID                     │ Device                   │ WiFi
     │                         ▼                          ▼
     │                    ┌─────────┐              ┌──────────┐
     └────────────────────│   PC    │◄────────────►│ Thetis   │
        (Output Report)   │  MIDI   │     LAN      │   SDR    │
                          └─────────┘     TCI      └──────────┘
```

### Debug & Status Indicators

**LED Configuration:**
- LED1: System status (ready/error/activity)
- LED2: MIDI activity indicator

**Debug Interface:**
- USB CDC serial on Device port (composite with MIDI)
- Optional external UART for secondary debug channel

### Hercules LED Control (Thetis SDR Integration)

**Primary Use Case: Bidirectional LED Control**

Control Hercules DJControl MP3 LE's **built-in LEDs** based on Thetis SDR radio state:
- TCI client on ESP32-C6 polls Thetis SDR status via WiFi
- Radio state changes trigger LED updates on controller
- LEDs controlled via USB **HID Output Reports** (21 bytes, Report ID 0x01)

**LED Mapping (from hdjmod Linux driver):**
- Play/Pause (Left/Right deck) - with blink support
- Master Tempo (Left/Right)
- Monitor
- Loop (Left/Right)
- Other control LEDs as documented in Protocol.txt

**Protocol Details:**
- **HID Output Report**: 21 bytes, Report ID 0x01
- Bit manipulation in HID buffer based on TCI state
- Example: Thetis TX state → Play_A LED ON

**Optional GPIO Expansion:**
- Additional P4 GPIO pins for external LEDs/relays if needed
- Secondary to primary HID LED control

### Power Management

**Primary Plan:**
- Power from PC via USB Device port
- ESP32-P4 provides power to Hercules via USB Host port

**Fallback:**
- External 5V supply if USB Host power insufficient for Hercules

## Software Architecture

### Technology Stack

**ESP32-P4 Firmware:**
- **Framework:** ESP-IDF (C pure)
- **USB Stack:** TinyUSB (host + device)
- **WiFi/Network:** esp_hosted (SDIO driver for C6 communication)
- **RTOS:** FreeRTOS (included in ESP-IDF)
- **Build System:** CMake + ESP-IDF toolchain

**ESP32-C6 Firmware:**
- **Framework:** ESP-IDF
- **Network:** esp_hosted device firmware
- **TCI Client:** Custom TCP client for Thetis SDR protocol
- **Communication:** SDIO slave to ESP32-P4

### Bidirectional Data Pipeline

**Controller → PC (MIDI Output):**
```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐     ┌─────────────┐
│ USB Host RX │ --> │  Hercules    │ --> │ Hook Chain  │ --> │ USB Device  │
│  (Hercules) │     │  → MIDI      │     │  Pipeline   │     │  TX (PC)    │
└─────────────┘     └──────────────┘     └─────────────┘     └─────────────┘
```

**Thetis → Controller (LED Control):**
```
┌──────────┐     ┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│ Thetis   │ --> │  ESP32-C6   │ --> │  ESP32-P4    │ --> │  Hercules   │
│ TCI/TCP  │     │ TCI Client  │     │ LED Mapping  │     │  HID Report │
└──────────┘     └─────────────┘     └──────────────┘     └─────────────┘
                      WiFi               SDIO             USB Host (HID)
```

**Buffer Strategy:**
- Static allocation only (no dynamic memory at runtime)
- 256 bytes RX/TX buffers for USB endpoints
- Ring buffers for inter-task communication via FreeRTOS queues

### Component Structure

**ESP32-P4 Project:**
```
/main
  ├── main.c                    // Entry point, FreeRTOS task creation
  ├── usb_host_hercules.c/h     // USB Host handling, Hercules parsing
  ├── usb_device_midi.c/h       // USB Device MIDI class + CDC debug
  ├── midi_converter.c/h        // Hercules protocol → MIDI mapping
  ├── led_controller.c/h        // Hercules LED control via HID Output Reports
  ├── hook_system.c/h           // Hook chain + built-in hooks
  ├── tci_bridge.c/h            // Bridge between C6 (via esp_hosted) and LED controller
  └── config.c/h                // NVS configuration management

/components
  ├── midi_types/               // Common MIDI definitions and types
  │   ├── midi_types.h
  │   └── CMakeLists.txt
  └── esp_hosted/               // ESP-Hosted SDIO driver (from Espressif)
      └── (esp_hosted library)
```

**ESP32-C6 Project:**
```
/c6_firmware
  ├── main.c                    // ESP-Hosted device + TCI client
  ├── tci_client.c/h            // Thetis TCI protocol client (TCP)
  ├── tci_parser.c/h            // Parse TCI responses, extract radio state
  └── wifi_manager.c/h          // WiFi connection management
```

### Task Architecture

**ESP32-P4 FreeRTOS Tasks:**

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

4. **LED Control Task** (Priority: Medium)
   - Receives TCI state updates from C6 (via esp_hosted)
   - Maps radio state → LED commands
   - Sends HID Output Reports to Hercules controller

5. **TCI Bridge Task** (Priority: Low)
   - Manages esp_hosted communication with C6
   - Forwards TCI state to LED Control task

**ESP32-C6 FreeRTOS Tasks:**

1. **WiFi Manager Task** (Priority: Medium)
   - WiFi connection management
   - Reconnection logic

2. **TCI Client Task** (Priority: Medium)
   - TCP connection to Thetis SDR
   - Poll TCI status (configurable interval, e.g., 100ms)
   - Parse responses, extract radio state

3. **esp_hosted Device Task** (Priority: High)
   - SDIO slave communication with P4
   - Forward TCI state updates to P4

**Inter-task Communication:**
- FreeRTOS queues with static allocation
- No direct shared memory (thread-safe design)
- P4↔C6: esp_hosted API (network-like interface over SDIO)

## LED Control System (Thetis SDR Integration)

### Overview

Bidirectional communication enables ESP32-P4 to **control Hercules controller LEDs** based on Thetis SDR radio state, providing visual feedback for radio operations directly on the DJ controller.

### TCI Protocol (Thetis Control Interface)

**Protocol Details:**
- **Transport:** TCP/IP (default port 50001)
- **Format:** Text-based command/response protocol
- **Polling:** ESP32-C6 polls Thetis status at configurable interval (e.g., 100ms)
- **Commands:** Query radio state (TX, RX, Mode, VFO, etc.)

**Example TCI Commands:**
```
tx:?              // Query TX status
rx:?              // Query RX status
mode:?            // Query operating mode
vfo:?             // Query VFO frequency
```

**Responses:**
```
tx:0,1;           // Radio 0, TX enabled
rx:0,0;           // Radio 0, RX disabled
mode:0,LSB;       // Radio 0, LSB mode
```

### LED Mapping Strategy

**Radio State → Hercules LED Mapping:**

| TCI State | Hercules LED | Function |
|-----------|--------------|----------|
| TX Active | Play_A | TX indicator |
| RX Active | Play_B | RX indicator |
| CW Mode | N1_A | CW mode active |
| LSB Mode | N2_A | LSB mode |
| USB Mode | N3_A | USB mode |
| Tune Mode | Loop_A | Tuning |
| Split Mode | Master_Tempo_A | Split operation |

**Configuration:** Mapping table stored in NVS, configurable via debug serial commands.

### HID Output Report Format

**Based on hdjmod Linux driver analysis:**

```c
typedef struct {
    uint8_t report_id;     // 0x01
    uint8_t led_data[20];  // Bit-mapped LED states
} hercules_led_report_t;
```

**LED Control Process:**
1. TCI state change detected on C6
2. State forwarded to P4 via esp_hosted
3. P4 maps TCI state → LED bitmap
4. HID Output Report constructed
5. Report sent to Hercules via USB Host

**Reference:** `midirender.c` from hdjmod for bit manipulation details.

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

**Note:** GPIO output hook removed - LED control is now handled via dedicated LED Control System (see previous section) using HID Output Reports to controller.

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

**General:**
- `status` - Show system state, enabled hooks, WiFi status, TCI connection
- `config save` - Persist configuration to NVS

**MIDI & Hooks:**
- `hook list` - List available hooks
- `hook enable <name>` - Enable hook
- `hook disable <name>` - Disable hook
- `midi dump` - Enable MIDI stream logging

**LED Control:**
- `led test <led_name>` - Test specific LED (e.g., `led test Play_A`)
- `led map show` - Show TCI→LED mapping table
- `led map set <tci_state> <led_name>` - Configure mapping

**TCI/WiFi:**
- `wifi status` - Show WiFi connection state
- `tci status` - Show TCI connection and last poll time
- `tci test` - Send test TCI command

## Performance Requirements

### Latency Targets

**MIDI Path (Controller → PC):**
- **End-to-end latency:** < 5ms (Hercules USB → PC MIDI out)
- **Hook processing:** < 1ms per message
- **Buffer depth:** Minimal (256 bytes = ~85 MIDI messages)

**LED Control Path (Thetis → Controller):**
- **TCI poll interval:** 100ms (configurable)
- **LED update latency:** < 50ms (TCI response → LED visible)
- **Acceptable for radio control feedback:** Human perception ~100ms

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

**MIDI Path:**
- USB Host enumeration with Hercules controller
- USB Device enumeration as MIDI class on PC
- End-to-end MIDI flow with real controller
- Hook chain with multiple enabled hooks

**LED Control Path:**
- ESP32-P4 ↔ ESP32-C6 communication via esp_hosted
- TCI client connection to Thetis SDR
- TCI state parsing and LED mapping
- HID Output Report transmission to Hercules
- End-to-end: Thetis TX → Controller LED visible

**Dual-chip Communication:**
- SDIO throughput and reliability testing
- P4/C6 state synchronization

### Performance Testing

- Latency measurement (oscilloscope or software timing)
- Sustained load (rapid button mashing)
- Edge cases (simultaneous multi-control input)

## Development Phases

### Phase 1: ESP32-P4 Core Infrastructure
- ESP-IDF project setup for P4
- TinyUSB integration (host + device)
- Basic USB enumeration (Hercules + PC)
- Static buffer allocation
- CDC debug serial functional

### Phase 2: MIDI Pipeline (P4)
- Port Hercules protocol conversion from Arduino
- MIDI message types and definitions
- Basic pipeline: USB Host RX → Convert → USB Device TX
- Validate MIDI output on PC (no hooks yet)

### Phase 3: ESP32-C6 Firmware
- ESP-IDF project setup for C6
- esp_hosted device firmware integration
- WiFi manager implementation
- Basic P4↔C6 communication via SDIO
- Validate network stack on C6

### Phase 4: TCI Client (C6)
- TCI protocol client implementation
- TCP connection to Thetis SDR
- TCI command/response parsing
- State extraction (TX, RX, Mode, etc.)
- Forward state to P4 via esp_hosted

### Phase 5: LED Control System (P4)
- Receive TCI state from C6
- Implement TCI→LED mapping table
- HID Output Report construction (21 bytes)
- USB Host HID write to Hercules
- Test with real controller LEDs

### Phase 6: Hook System & Refinement
- Hook registration framework
- Built-in hook implementations
- NVS configuration support (hooks + LED mapping)
- Serial debug commands (full set)
- End-to-end testing and optimization

## Open Questions

1. **ESP32-P4 USB Host/Device simultaneous mode**: Verify TinyUSB supports both modes concurrently on P4
2. **Power budget**: Confirm GUITION board can provide sufficient USB Host current for Hercules
3. **Hercules HID LED protocol**: Complete LED bit mapping (partial info from hdjmod, may need testing)
4. **esp_hosted SDIO configuration**: Verify GUITION board SDIO pin mapping and initialization
5. **TCI protocol details**: Verify complete TCI command set for all needed radio states
6. **C6 firmware flashing**: Determine if C6 firmware must be flashed separately or can be updated from P4

## References

**Hardware:**
- [GUITION ESP32-P4 + ESP32-C6 Board](https://www.cnx-software.com/2025/07/18/14-development-board-features-guition-esp32-p4-esp32-c6-module/)
- [ESP32-P4 datasheet](https://www.espressif.com/en/products/socs/esp32-p4)
- [ESP32-C6 datasheet](https://www.espressif.com/en/products/socs/esp32-c6)

**Hercules Protocol:**
- [Original Teensy implementation](https://github.com/pr8x/djcontrolmp3le_usb_proxy)
- [djcontrol protocol research](https://github.com/foomatic/djcontrol)
- [hdjmod Linux driver](https://github.com/bkero/hdjmod) - LED control via HID
- Protocol.txt (in this repository)

**Software Frameworks:**
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [TinyUSB documentation](https://docs.tinyusb.org/)
- [esp_hosted GitHub](https://github.com/espressif/esp-hosted) - P4↔C6 communication

**Thetis SDR:**
- [Thetis SDR](https://github.com/TAPR/OpenHPSDR-Thetis)
- TCI Protocol documentation (TBD - may need reverse engineering)

## Success Criteria

**MIDI Path (Controller → PC):**
- [ ] Hercules controller recognized and functional on modern OS without drivers
- [ ] MIDI mapping matches or exceeds original Teensy implementation
- [ ] End-to-end latency < 5ms (button press → MIDI on PC)
- [ ] At least 3 useful built-in hooks implemented
- [ ] All controller inputs (buttons, knobs, jogs) correctly mapped

**LED Control Path (Thetis → Controller):**
- [ ] ESP32-C6 connects to WiFi and establishes TCI connection to Thetis
- [ ] TCI state changes (TX, RX, Mode) correctly detected
- [ ] Hercules controller LEDs respond to Thetis state within 100ms
- [ ] At least 5 radio states mapped to controller LEDs
- [ ] LED control does not interfere with MIDI latency

**System Integration:**
- [ ] ESP32-P4 and ESP32-C6 communicate reliably via esp_hosted/SDIO
- [ ] Debug interface functional for troubleshooting (CDC serial)
- [ ] Configuration persists across reboots (NVS)
- [ ] Clean, maintainable C codebase with documentation
- [ ] Both P4 and C6 firmware can be built and flashed independently

## Future Enhancements (Out of Scope for V1)

- Support for additional DJ controllers (other Hercules models, Pioneer, etc.)
- Web-based configuration interface (served by ESP32-C6)
- MQTT integration for remote monitoring/control
- Advanced hook scripting (Lua/WASM bytecode execution)
- MIDI learn mode for custom mappings
- Bi-directional MIDI: PC software → Hercules LED (not just Thetis)
- OTA firmware updates for both P4 and C6
- TCI protocol extensions (PTT control, frequency setting via controller knobs)
- Multi-radio support (multiple Thetis instances)
