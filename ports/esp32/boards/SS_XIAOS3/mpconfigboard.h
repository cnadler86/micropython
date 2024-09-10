#ifndef MICROPY_HW_BOARD_NAME
// Can be set by mpconfigboard.cmake.
#define MICROPY_HW_BOARD_NAME               "XIAO ESP32S3 sense"
#endif
#define MICROPY_HW_MCU_NAME                 "ESP32S3"

#define MICROPY_PY_MACHINE_DAC              (0)

// Enable UART REPL for modules that have an external USB-UART and don't use native USB.
#define MICROPY_HW_ENABLE_UART_REPL         (0)

#define MICROPY_HW_I2C0_SCL                 (5)
#define MICROPY_HW_I2C0_SDA                 (4)

#define MP_CAMERA_MODULE_ENABLED            (1)

#define MICROPY_CAMERA_PINS_DATA   ({15,17,18,16,14,12,11,48})
#define MICROPY_CAMERA_PIN_PCLK    (13)
#define MICROPY_CAMERA_PIN_VSYNC   (38)
#define MICROPY_CAMERA_PIN_HREF    (47)
#define MICROPY_CAMERA_PIN_XCLK    (10)
#define MICROPY_CAMERA_PIN_PWDN    (-1)
#define MICROPY_CAMERA_PIN_RESET   (-1)
#define MICROPY_CAMERA_PIN_SIOD    (40)
#define MICROPY_CAMERA_PIN_SIOC    (39)