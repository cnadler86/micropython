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

#define MICROPY_CAMERA_MODEL_XIAO_ESP32S3    (1)
