# ESP32-P4 USB MIDI Proxy - Implementation Task Breakdown

**Date:** 2025-11-17
**Status:** Ready for Implementation
**Reference:** [PRD](./2025-11-17-esp32p4-usb-midi-proxy-design.md)

---

## Phase 1: ESP32-P4 Core Infrastructure

**Goal:** Get ESP32-P4 development environment ready with basic USB functionality

### 1.1 Development Environment Setup
- [ ] Install ESP-IDF v5.3+ for ESP32-P4 support
- [ ] Configure toolchain and PATH
- [ ] Test basic "hello world" build for ESP32-P4
- [ ] Set up GUITION board USB connection and flash utility
- [ ] Verify serial monitor works (115200 baud)

### 1.2 Project Structure Creation
- [ ] Create ESP-IDF project: `esp32_p4_midi_proxy/`
- [ ] Initialize git repository (if separate from current)
- [ ] Create directory structure from PRD:
  - [ ] `main/` with empty .c/.h files
  - [ ] `components/midi_types/`
  - [ ] `components/esp_hosted/` (placeholder)
- [ ] Configure `CMakeLists.txt` for main project
- [ ] Configure `sdkconfig` for ESP32-P4

### 1.3 TinyUSB Integration
- [ ] Add TinyUSB component to project
- [ ] Configure USB OTG1 as **Host mode** in sdkconfig
- [ ] Configure USB OTG2 as **Device mode** in sdkconfig
- [ ] Create test: USB Host detects any USB device
- [ ] Create test: USB Device appears on PC (basic VID/PID)
- [ ] Verify both USB ports work simultaneously

### 1.4 Static Buffer Allocation
- [ ] Define buffer sizes in `config.h`:
  - [ ] USB Host RX: 256 bytes
  - [ ] USB Device TX: 256 bytes
  - [ ] FreeRTOS queue depth: 16 messages
- [ ] Allocate static buffers (no malloc in code)
- [ ] Create utility functions for buffer management

### 1.5 CDC Debug Serial
- [ ] Implement USB composite device (MIDI + CDC)
- [ ] Create CDC serial interface on USB Device port
- [ ] Test serial output: `printf()` → CDC serial
- [ ] Implement basic command parser (for future debug commands)
- [ ] Test: send "status" command, get response

**Phase 1 Completion Criteria:**
- [ ] Can flash firmware to ESP32-P4
- [ ] Both USB ports initialized correctly
- [ ] CDC debug serial functional on PC
- [ ] Basic project structure in place

---

## Phase 2: MIDI Pipeline (ESP32-P4)

**Goal:** Implement Hercules → MIDI → PC data flow

### 2.1 MIDI Types Component
- [ ] Create `components/midi_types/midi_types.h`
- [ ] Define MIDI message structure:
  ```c
  typedef struct {
      uint8_t status;
      uint8_t data1;
      uint8_t data2;
  } midi_message_t;
  ```
- [ ] Define MIDI constants (Note On, Note Off, CC, etc.)
- [ ] Create helper functions: `midi_create_note_on()`, etc.
- [ ] Write unit tests for MIDI message creation

### 2.2 USB Host Hercules Driver
- [ ] Create `usb_host_hercules.c/h`
- [ ] Port device detection from Arduino code:
  - [ ] VID: 0x06f8, PID: 0xb105
- [ ] Implement init sequence (from djcontrolmp3le_usb_proxy.ino):
  - [ ] 12 control transfers (lines 30-45)
- [ ] Create bulk IN pipe (endpoint 0x81, 64 bytes)
- [ ] Implement state read (38-byte device state)
- [ ] Test: Hercules controller detected and initialized

### 2.3 Hercules Protocol Conversion
- [ ] Create `midi_converter.c/h`
- [ ] Port control mapping table from Arduino:
  - [ ] All 46 buttons (lines 149-196)
  - [ ] All 9 dials/sliders (lines 198-207)
  - [ ] All 4 jog wheels (lines 211-214)
- [ ] Implement button → MIDI Note conversion
- [ ] Implement dial → MIDI CC conversion
- [ ] Implement jog → MIDI CC (wrap-around handling)
- [ ] Test with controller: press button → correct MIDI note

### 2.4 USB Device MIDI Class
- [ ] Create `usb_device_midi.c/h`
- [ ] Implement TinyUSB MIDI device descriptor
- [ ] Configure as class-compliant MIDI device
- [ ] Implement MIDI TX function: `midi_send(midi_message_t* msg)`
- [ ] Test on PC: device appears as MIDI input
- [ ] Test: send MIDI note → MIDI monitoring software shows it

### 2.5 FreeRTOS Task Pipeline
- [ ] Create USB Host Task:
  - [ ] Read Hercules state
  - [ ] Detect changes
  - [ ] Convert to MIDI
  - [ ] Push to queue
- [ ] Create USB Device Task:
  - [ ] Pop from queue
  - [ ] Send via USB MIDI
- [ ] Configure task priorities (both High)
- [ ] Create FreeRTOS queue (16 messages, static allocation)
- [ ] Test end-to-end: button press → MIDI on PC

### 2.6 Validation & Testing
- [ ] Test all 46 buttons
- [ ] Test all 9 dials/sliders
- [ ] Test all 4 jog wheels
- [ ] Measure latency (button → MIDI visible on PC)
- [ ] Test burst input (rapid button presses)
- [ ] Verify no buffer overflows

**Phase 2 Completion Criteria:**
- [ ] Hercules controller fully functional as MIDI device
- [ ] All inputs correctly mapped
- [ ] Latency < 5ms
- [ ] Stable under load

---

## Phase 3: ESP32-C6 Firmware

**Goal:** Get ESP32-C6 communicating with ESP32-P4 via SDIO

### 3.1 C6 Development Environment
- [ ] Create separate project: `esp32_c6_tci_client/`
- [ ] Configure ESP-IDF for ESP32-C6
- [ ] Test basic build and flash for C6
- [ ] Verify C6 boots independently

### 3.2 esp_hosted Integration (C6 Side)
- [ ] Clone esp_hosted from Espressif GitHub
- [ ] Build esp_hosted **slave** firmware for C6
- [ ] Configure SDIO slave pins for GUITION board:
  - [ ] CLK, CMD, D0, D1, D2, D3 pins
  - [ ] Verify pin mapping from GUITION schematic
- [ ] Flash esp_hosted to C6
- [ ] Test: C6 boots and waits for SDIO master

### 3.3 WiFi Manager Implementation
- [ ] Create `wifi_manager.c/h`
- [ ] Implement WiFi station mode
- [ ] Add NVS storage for WiFi credentials:
  - [ ] SSID
  - [ ] Password
- [ ] Implement connection logic with retry
- [ ] Add reconnection on disconnect
- [ ] Test: C6 connects to WiFi network
- [ ] Test: survives WiFi disconnect/reconnect

### 3.4 esp_hosted Integration (P4 Side)
- [ ] Add esp_hosted **master** component to P4 project
- [ ] Configure SDIO master pins for GUITION board
- [ ] Initialize SDIO communication
- [ ] Test: P4 detects C6 via SDIO
- [ ] Test: P4 can use C6 for network (ping test)

### 3.5 Network Stack Validation
- [ ] Implement simple TCP echo client on C6
- [ ] Test TCP connection to local server
- [ ] Measure SDIO throughput (P4 ↔ C6)
- [ ] Verify network stability under load
- [ ] Test: simultaneous WiFi + SDIO traffic

**Phase 3 Completion Criteria:**
- [ ] C6 firmware builds and runs
- [ ] C6 connects to WiFi reliably
- [ ] P4 ↔ C6 communication via SDIO works
- [ ] Network stack functional via esp_hosted

---

## Phase 4: TCI Client (ESP32-C6)

**Goal:** Connect to Thetis SDR and parse radio state

### 4.1 TCI Protocol Research
- [ ] Set up Thetis SDR for testing
- [ ] Enable TCI server in Thetis (port 50001)
- [ ] Capture TCI traffic with Wireshark
- [ ] Document TCI commands needed:
  - [ ] `tx:?` - TX status
  - [ ] `rx:?` - RX status
  - [ ] `mode:?` - Operating mode
  - [ ] Response format verification
- [ ] Create TCI protocol reference doc

### 4.2 TCI Client Implementation
- [ ] Create `tci_client.c/h`
- [ ] Implement TCP connection to Thetis:
  - [ ] Configurable IP address (NVS)
  - [ ] Configurable port (default 50001)
- [ ] Implement TCI command send function
- [ ] Implement TCI response receive function
- [ ] Add connection retry logic
- [ ] Test: connect to Thetis, send `tx:?`, receive response

### 4.3 TCI Parser Implementation
- [ ] Create `tci_parser.c/h`
- [ ] Define radio state structure:
  ```c
  typedef struct {
      bool tx_active;
      bool rx_active;
      char mode[8];  // "LSB", "USB", "CW", etc.
      // ... more fields
  } radio_state_t;
  ```
- [ ] Implement parsers for each TCI command:
  - [ ] Parse `tx:0,1;` → tx_active = true
  - [ ] Parse `mode:0,LSB;` → mode = "LSB"
- [ ] Test each parser function
- [ ] Test: poll Thetis, extract all needed states

### 4.4 TCI Polling Task
- [ ] Create FreeRTOS task for TCI polling
- [ ] Implement polling loop (100ms interval, configurable)
- [ ] Send TCI queries sequentially
- [ ] Parse responses, update radio_state
- [ ] Detect state changes (compare old vs new)
- [ ] Test: state changes logged to serial

### 4.5 State Forwarding to P4
- [ ] Define state message format for esp_hosted
- [ ] Implement serialization: `radio_state_t` → byte array
- [ ] Send state updates to P4 via esp_hosted API
- [ ] Add sequence number to detect missed updates
- [ ] Test: change TX on Thetis → P4 receives update

**Phase 4 Completion Criteria:**
- [ ] TCI client connects to Thetis reliably
- [ ] All required radio states extracted
- [ ] State changes forwarded to P4 via SDIO
- [ ] Polling works continuously without errors

---

## Phase 5: LED Control System (ESP32-P4)

**Goal:** Control Hercules LEDs based on TCI state from C6

### 5.1 TCI Bridge Component
- [ ] Create `tci_bridge.c/h`
- [ ] Implement esp_hosted receive handler
- [ ] Deserialize state messages from C6
- [ ] Create FreeRTOS queue for LED commands
- [ ] Forward state changes to LED Control task
- [ ] Test: receive state from C6 → queue has data

### 5.2 LED Mapping Table
- [ ] Create `led_controller.c/h`
- [ ] Define LED mapping structure:
  ```c
  typedef struct {
      char tci_state[16];   // "TX_ACTIVE", "MODE_LSB", etc.
      uint8_t led_byte;     // Which byte in HID report
      uint8_t led_bit;      // Which bit in that byte
  } led_mapping_t;
  ```
- [ ] Implement default mappings from PRD:
  - [ ] TX Active → Play_A
  - [ ] RX Active → Play_B
  - [ ] CW Mode → N1_A
  - [ ] LSB Mode → N2_A
  - [ ] USB Mode → N3_A
- [ ] Store mappings in NVS
- [ ] Implement mapping lookup function

### 5.3 HID Output Report Construction
- [ ] Research hdjmod `midirender.c` for bit positions
- [ ] Define HID report structure:
  ```c
  typedef struct {
      uint8_t report_id;     // 0x01
      uint8_t led_data[20];
  } hercules_led_report_t;
  ```
- [ ] Implement report builder:
  - [ ] Clear report buffer
  - [ ] Set bits based on radio state
  - [ ] Handle blink states (if needed)
- [ ] Test: build report for "TX Active"

### 5.4 USB Host HID Write
- [ ] Add HID output capability to USB Host driver
- [ ] Implement HID SET_REPORT control transfer
- [ ] Send HID report to Hercules controller
- [ ] Test with known LED (e.g., Play_A)
- [ ] Verify LED turns on/off correctly

### 5.5 LED Control Task
- [ ] Create FreeRTOS task (Priority: Medium)
- [ ] Receive state updates from TCI Bridge
- [ ] Map state → LED bitmap
- [ ] Construct HID report
- [ ] Send to Hercules via USB Host
- [ ] Add rate limiting (avoid flooding USB)
- [ ] Test end-to-end: change TX in Thetis → LED lights up

### 5.6 LED Testing & Refinement
- [ ] Test all mapped LEDs individually
- [ ] Verify LEDs match expected radio states
- [ ] Test rapid state changes
- [ ] Measure LED update latency (< 100ms target)
- [ ] Handle edge cases (Thetis disconnect, etc.)
- [ ] Document final bit mappings for all LEDs

**Phase 5 Completion Criteria:**
- [ ] TCI state updates received from C6
- [ ] At least 5 radio states mapped to LEDs
- [ ] LEDs respond correctly to Thetis state
- [ ] Latency < 100ms
- [ ] LED control doesn't affect MIDI latency

---

## Phase 6: Hook System & Refinement

**Goal:** Add extensibility and polish the system

### 6.1 Hook System Framework
- [ ] Create `hook_system.c/h`
- [ ] Define hook function signature:
  ```c
  typedef bool (*midi_hook_t)(midi_message_t* msg);
  ```
- [ ] Implement hook registration:
  - [ ] Static array of hooks (MAX_HOOKS = 16)
  - [ ] `hook_register(name, function, enabled)`
- [ ] Implement hook chain execution
- [ ] Add to MIDI pipeline (between converter and USB TX)
- [ ] Test: dummy hook that logs messages

### 6.2 Built-in Hooks Implementation
- [ ] **Velocity Adjustment Hook:**
  - [ ] Amplify/reduce note velocity
  - [ ] Configurable gain (NVS)
  - [ ] Test: note velocity modified
- [ ] **Note Remapping Hook:**
  - [ ] Transpose notes by N semitones
  - [ ] Configurable offset (NVS)
  - [ ] Test: notes shifted correctly
- [ ] **CC Filter Hook:**
  - [ ] Block specific CC numbers
  - [ ] Configurable filter list (NVS)
  - [ ] Test: filtered CCs dropped
- [ ] **MIDI Logger Hook:**
  - [ ] Log messages to CDC serial
  - [ ] Enable/disable via command
  - [ ] Test: MIDI dump visible

### 6.3 NVS Configuration System
- [ ] Create `config.c/h`
- [ ] Define config structure:
  ```c
  typedef struct {
      bool hooks_enabled[MAX_HOOKS];
      int8_t velocity_offset;
      uint8_t note_transpose;
      led_mapping_t led_map[MAX_LED_MAPPINGS];
      char wifi_ssid[32];
      char wifi_password[64];
      char thetis_ip[16];
      uint16_t tci_poll_interval_ms;
  } system_config_t;
  ```
- [ ] Implement NVS read/write functions
- [ ] Load config on boot
- [ ] Save config on command
- [ ] Test: config survives reboot

### 6.4 Debug Command System
- [ ] Extend CDC command parser
- [ ] Implement commands:
  - [ ] `status` - Show system state
  - [ ] `hook list` - List hooks
  - [ ] `hook enable <name>` - Enable hook
  - [ ] `hook disable <name>` - Disable hook
  - [ ] `midi dump` - Toggle MIDI logging
  - [ ] `led test <name>` - Test specific LED
  - [ ] `led map show` - Show LED mappings
  - [ ] `led map set <state> <led>` - Set mapping
  - [ ] `wifi status` - WiFi connection state
  - [ ] `tci status` - TCI connection state
  - [ ] `tci test` - Send test TCI command
  - [ ] `config save` - Persist to NVS
- [ ] Test each command
- [ ] Create help text

### 6.5 End-to-End Testing
- [ ] **MIDI Path Test:**
  - [ ] All Hercules inputs → correct MIDI
  - [ ] Latency measurement
  - [ ] Stress test (rapid inputs)
  - [ ] Hook chain with all hooks enabled
- [ ] **LED Path Test:**
  - [ ] All TCI states → correct LEDs
  - [ ] Latency measurement
  - [ ] WiFi disconnect recovery
  - [ ] Thetis disconnect recovery
- [ ] **Integration Test:**
  - [ ] Simultaneous MIDI + LED control
  - [ ] No interference between paths
  - [ ] Power cycle recovery
  - [ ] Config persistence

### 6.6 Documentation & Cleanup
- [ ] Write user manual:
  - [ ] Hardware setup
  - [ ] Flashing firmware (P4 and C6)
  - [ ] WiFi configuration
  - [ ] LED mapping customization
  - [ ] Debug commands reference
- [ ] Code cleanup:
  - [ ] Remove debug prints
  - [ ] Add comments
  - [ ] Check for memory leaks
  - [ ] Optimize buffer usage
- [ ] Create build instructions
- [ ] Write troubleshooting guide

**Phase 6 Completion Criteria:**
- [ ] All hooks implemented and tested
- [ ] All debug commands functional
- [ ] Configuration system complete
- [ ] Full system tested end-to-end
- [ ] Documentation complete

---

## Testing Checklist

### Hardware Requirements
- [ ] GUITION ESP32-P4 + C6 board
- [ ] Hercules DJControl MP3 LE controller
- [ ] PC with USB port
- [ ] WiFi network
- [ ] PC running Thetis SDR

### Software Requirements
- [ ] ESP-IDF v5.3+
- [ ] MIDI monitoring software (e.g., MIDI Monitor, MIDIberry)
- [ ] Thetis SDR with TCI enabled
- [ ] Serial terminal (e.g., PuTTY, screen)
- [ ] Wireshark (for TCI debugging)

### Test Scenarios
- [ ] Cold boot test (power on from off)
- [ ] Warm reboot test (soft reset)
- [ ] WiFi disconnect/reconnect
- [ ] Thetis disconnect/reconnect
- [ ] Hercules disconnect/reconnect
- [ ] Simultaneous operations (all inputs + LED control)
- [ ] 24-hour stability test

---

## Success Metrics

### Performance
- [ ] MIDI latency: < 5ms (measured)
- [ ] LED latency: < 100ms (measured)
- [ ] SDIO throughput: > 10 Mbps (measured)
- [ ] No dropped MIDI messages under stress
- [ ] CPU usage: < 50% average (both P4 and C6)

### Functionality
- [ ] 100% of Hercules inputs working
- [ ] 100% of target LED mappings working
- [ ] WiFi reconnect: < 10 seconds
- [ ] TCI reconnect: < 5 seconds
- [ ] Config persistence: 100% reliable

### Code Quality
- [ ] No compiler warnings
- [ ] No memory leaks (valgrind equivalent)
- [ ] Stack usage within limits
- [ ] All TODOs resolved
- [ ] Code reviewed and commented

---

## Timeline Estimate (Rough)

- **Phase 1:** 2-3 days
- **Phase 2:** 3-4 days
- **Phase 3:** 3-4 days
- **Phase 4:** 2-3 days
- **Phase 5:** 3-4 days
- **Phase 6:** 3-4 days

**Total:** ~16-22 days (3-4 weeks)

*Note: Assumes full-time development and hardware availability*
