/*
 * CDC Debug Console Implementation
 */

#include "cdc_console.h"
#include "usb_device.h"
#include "usb_host.h"
#include "esp_log.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static const char *TAG = "cdc_console";

#define CMD_BUFFER_SIZE 128
#define MAX_ARGS 8

static char cmd_buffer[CMD_BUFFER_SIZE];
static uint16_t cmd_buffer_pos = 0;

//--------------------------------------------------------------------+
// Helper Functions
//--------------------------------------------------------------------+

static void execute_command(const char *cmd_line);
static void cmd_status(int argc, char *argv[]);
static void cmd_help(int argc, char *argv[]);
static void cmd_reset(int argc, char *argv[]);

//--------------------------------------------------------------------+
// Initialization
//--------------------------------------------------------------------+

void cdc_console_init(void)
{
    ESP_LOGI(TAG, "CDC Console initialized");
    cmd_buffer_pos = 0;
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
}

//--------------------------------------------------------------------+
// Console Output
//--------------------------------------------------------------------+

void cdc_console_printf(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0) {
        cdc_printf("%s", buffer);
    }
}

void cdc_console_print_help(void)
{
    cdc_console_printf("\r\n");
    cdc_console_printf("=== ESP32-P4 USB MIDI Proxy - Debug Console ===\r\n");
    cdc_console_printf("\r\n");
    cdc_console_printf("Available commands:\r\n");
    cdc_console_printf("  status     - Show system status\r\n");
    cdc_console_printf("  help       - Show this help message\r\n");
    cdc_console_printf("  reset      - Soft reset the system\r\n");
    cdc_console_printf("\r\n");
    cdc_console_printf("Future commands (not yet implemented):\r\n");
    cdc_console_printf("  hook list              - List all hooks\r\n");
    cdc_console_printf("  hook enable <name>     - Enable hook\r\n");
    cdc_console_printf("  hook disable <name>    - Disable hook\r\n");
    cdc_console_printf("  midi dump              - Toggle MIDI logging\r\n");
    cdc_console_printf("  led test <name>        - Test specific LED\r\n");
    cdc_console_printf("  led map show           - Show LED mappings\r\n");
    cdc_console_printf("  config save            - Save configuration to NVS\r\n");
    cdc_console_printf("\r\n");
}

//--------------------------------------------------------------------+
// Command Processing
//--------------------------------------------------------------------+

void cdc_console_process(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        uint8_t ch = data[i];

        // Echo character (with basic line editing)
        if (ch == '\r' || ch == '\n') {
            cdc_printf("\r\n");

            // Execute command if buffer is not empty
            if (cmd_buffer_pos > 0) {
                cmd_buffer[cmd_buffer_pos] = '\0';
                execute_command(cmd_buffer);
                cmd_buffer_pos = 0;
                memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
            }

            cdc_printf("> ");
        } else if (ch == '\b' || ch == 0x7F) {
            // Backspace
            if (cmd_buffer_pos > 0) {
                cmd_buffer_pos--;
                cdc_printf("\b \b");
            }
        } else if (ch >= 32 && ch < 127) {
            // Printable character
            if (cmd_buffer_pos < CMD_BUFFER_SIZE - 1) {
                cmd_buffer[cmd_buffer_pos++] = ch;
                cdc_printf("%c", ch);
            }
        }
    }
}

static void execute_command(const char *cmd_line)
{
    char *argv[MAX_ARGS];
    int argc = 0;
    char *token;
    char cmd_copy[CMD_BUFFER_SIZE];

    // Copy command line for tokenization
    strncpy(cmd_copy, cmd_line, CMD_BUFFER_SIZE - 1);
    cmd_copy[CMD_BUFFER_SIZE - 1] = '\0';

    // Tokenize command line
    token = strtok(cmd_copy, " \t");
    while (token != NULL && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }

    if (argc == 0) {
        return;
    }

    // Execute command
    if (strcmp(argv[0], "status") == 0) {
        cmd_status(argc, argv);
    } else if (strcmp(argv[0], "help") == 0) {
        cmd_help(argc, argv);
    } else if (strcmp(argv[0], "reset") == 0) {
        cmd_reset(argc, argv);
    } else {
        cdc_console_printf("Unknown command: %s\r\n", argv[0]);
        cdc_console_printf("Type 'help' for available commands.\r\n");
    }
}

//--------------------------------------------------------------------+
// Command Implementations
//--------------------------------------------------------------------+

static void cmd_status(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    cdc_console_printf("\r\n=== System Status ===\r\n");
    cdc_console_printf("Firmware Version: 0.1.0\r\n");
    cdc_console_printf("Chip: ESP32-P4\r\n");
    cdc_console_printf("\r\n");

    cdc_console_printf("USB Status:\r\n");
    cdc_console_printf("  Hercules: %s\r\n",
                      is_hercules_mounted() ? "Connected" : "Disconnected");
    cdc_console_printf("  MIDI Device: Active\r\n");
    cdc_console_printf("  CDC Serial: Active\r\n");
    cdc_console_printf("\r\n");

    // FreeRTOS task stats
    cdc_console_printf("FreeRTOS:\r\n");
    cdc_console_printf("  Free Heap: %lu bytes\r\n", esp_get_free_heap_size());
    cdc_console_printf("  Min Free Heap: %lu bytes\r\n", esp_get_minimum_free_heap_size());
    cdc_console_printf("\r\n");
}

static void cmd_help(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    cdc_console_print_help();
}

static void cmd_reset(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    cdc_console_printf("Resetting system in 2 seconds...\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
}
