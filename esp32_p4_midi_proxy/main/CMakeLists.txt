idf_component_register(SRCS "main.c"
                              "usb_host.c"
                              "usb_device.c"
                              "usb_descriptors.c"
                              "buffers.c"
                              "cdc_console.c"
                    INCLUDE_DIRS "."
                    REQUIRES midi_types
                    PRIV_REQUIRES "espressif__tinyusb")
