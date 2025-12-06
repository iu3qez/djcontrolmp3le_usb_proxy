/*
 * USB Host Header
 */

#ifndef USB_HOST_H
#define USB_HOST_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize USB Host (OTG1)
 */
void usb_host_init(void);

/**
 * USB Host task (should run in FreeRTOS task)
 */
void usb_host_task(void *pvParameters);

/**
 * Check if Hercules controller is mounted
 * @return true if Hercules is connected
 */
bool is_hercules_mounted(void);

#ifdef __cplusplus
}
#endif

#endif // USB_HOST_H
