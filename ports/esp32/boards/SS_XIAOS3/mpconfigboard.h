#ifndef MICROPY_HW_BOARD_NAME
// Can be set by mpconfigboard.cmake.
#define MICROPY_HW_BOARD_NAME               "XIAO ESP32S3 sense"
#endif
#define MICROPY_HW_MCU_NAME                 "ESP32S3"

#define MICROPY_PY_MACHINE_DAC              (0)

// Enable UART REPL for modules that have an external USB-UART and don't use native USB.
#define MICROPY_HW_ENABLE_UART_REPL         (0)

#define MICROPY_HW_I2C0_SCL                 (6)
#define MICROPY_HW_I2C0_SDA                 (5)

#define MICROPY_HW_SPI1_MOSI                (9)
#define MICROPY_HW_SPI1_MISO                (8)
#define MICROPY_HW_SPI1_SCK                 (7)

#define MP_CAMERA_MODULE_ENABLED    (1)
#define MICROPY_CAMERA_PIN_D0       (15)
#define MICROPY_CAMERA_PIN_D1       (17)
#define MICROPY_CAMERA_PIN_D2       (18)
#define MICROPY_CAMERA_PIN_D3       (16)
#define MICROPY_CAMERA_PIN_D4       (14)
#define MICROPY_CAMERA_PIN_D5       (12)
#define MICROPY_CAMERA_PIN_D6       (11)
#define MICROPY_CAMERA_PIN_D7       (48)
#define MICROPY_CAMERA_PIN_PCLK     (13)
#define MICROPY_CAMERA_PIN_VSYNC    (38)
#define MICROPY_CAMERA_PIN_HREF     (47)
#define MICROPY_CAMERA_PIN_XCLK     (10)
#define MICROPY_CAMERA_PIN_PWDN     (-1)
#define MICROPY_CAMERA_PIN_RESET    (-1)
#define MICROPY_CAMERA_PIN_SIOD     (40)
#define MICROPY_CAMERA_PIN_SIOC     (39)
#define MICROPY_CAMERA_XCLK_FREQ    (20)
#define MICROPY_CAMERA_FB_COUNT     (2)
#define MICROPY_CAMERA_JPEG_QUALITY (10)
#define MICROPY_CAMERA_GRAB_MODE    (1)