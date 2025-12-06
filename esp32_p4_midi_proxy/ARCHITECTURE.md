# ESP32-P4 USB MIDI Proxy - Architecture

## System Overview

The ESP32-P4 USB MIDI Proxy converts a Hercules DJControl MP3 LE controller into a class-compliant MIDI device. The system uses dual USB OTG ports and FreeRTOS tasks for concurrent processing.

## Task Architecture

### Task Pipeline

```
┌─────────────────┐
│  USB Host Task  │  (Priority: High)
│   (usb_host)    │  Polls Hercules controller via USB OTG1
└────────┬────────┘
         │ HID Reports (38 bytes)
         ▼
┌─────────────────┐
│ Report Callback │  Processes in USB Host task context
│ tuh_hid_report  │  - Compares with previous state
│   _received_cb  │  - Detects changes
└────────┬────────┘
         │ State Changes
         ▼
┌─────────────────┐
│ MIDI Converter  │  Runs in USB Host task context
│ midi_converter  │  - 46 Buttons → MIDI Notes
│   _process()    │  - 9 Dials → MIDI CC
│                 │  - 4 Jogs → MIDI CC
└────────┬────────┘
         │ MIDI Messages
         ▼
┌─────────────────┐
│  MIDI Queue     │  FreeRTOS Queue (16 messages deep)
│ (static alloc)  │  Decouples producer from consumer
└────────┬────────┘
         │ MIDI Messages
         ▼
┌─────────────────┐
│  MIDI TX Task   │  (Priority: High)
│   (midi_tx)     │  Consumes from queue
└────────┬────────┘
         │ USB MIDI Packets
         ▼
┌─────────────────┐
│ USB Device Task │  (Priority: High)
│  (usb_device)   │  TinyUSB device stack
│                 │  Sends via USB OTG2
└─────────────────┘
```

### Task Details

| Task | Priority | Stack | Function |
|------|----------|-------|----------|
| `usb_host` | High (configMAX_PRIORITIES - 2) | 4096 | USB Host stack, Hercules polling |
| `usb_device` | High (configMAX_PRIORITIES - 2) | 4096 | USB Device stack, MIDI + CDC |
| `midi_tx` | High (configMAX_PRIORITIES - 3) | 3072 | Queue consumer, MIDI TX |

## Data Flow

### Input Path (Hercules → MIDI)

1. **USB Host Task** polls Hercules controller at ~1ms intervals
2. **HID Report Callback** receives 38-byte state report
3. **State Comparison** detects button/dial/jog changes
4. **MIDI Converter** generates MIDI messages for changes
5. **MIDI Queue** buffers messages (static allocation)
6. **MIDI TX Task** consumes and sends via USB Device

### Output Path (MIDI → PC)

1. **MIDI TX Task** blocks on queue
2. Receives MIDI message from queue
3. Calls `midi_send()` → TinyUSB MIDI device
4. **USB Device Task** handles USB transmission

### Debug Path (CDC Serial)

1. **USB Device Task** receives CDC data
2. **CDC Console** parses commands
3. Commands access stats, control system
4. Responses sent via `cdc_printf()`

## Memory Architecture

### Static Buffers

All buffers are statically allocated (no malloc):

```c
// USB buffers
usb_host_buffer_t g_usb_host_rx_buffer;        // 256 bytes
usb_device_buffer_t g_usb_device_tx_buffer;    // 256 bytes

// Hercules state
hercules_state_t g_hercules_state;             // 76 bytes (38×2)

// MIDI queue
StaticQueue_t g_midi_queue_struct;
uint8_t g_midi_queue_storage[16 * sizeof(midi_message_t)];  // 48 bytes
```

**Total static allocation:** ~636 bytes

### Control Mapping

- **46 Buttons** → MIDI Notes 0-45 (Channel 1)
- **9 Dials/Sliders** → MIDI CC 46-54 (Channel 1)
- **4 Jog Wheels** → MIDI CC 55-58 (Channel 1)

## Performance Targets

| Metric | Target | Achieved |
|--------|--------|----------|
| MIDI Latency | < 5ms | TBD |
| USB Poll Rate | ~1ms | TBD |
| Queue Depth | 16 messages | ✓ |
| CPU Usage | < 50% | TBD |

## Configuration

See `main/config.h` for all tunable parameters:
- Buffer sizes
- Queue depths
- Task priorities
- Poll intervals

## Debug Interface

CDC Serial commands:
- `status` - Show system stats (MIDI messages, queue depth, heap)
- `midi` - Reset MIDI statistics
- `help` - Show all commands
- `reset` - Soft reset system

## Future Extensions

- Hook system (Phase 6) for MIDI message processing
- LED control (Phase 5) for visual feedback
- NVS configuration persistence
- Custom MIDI mappings
