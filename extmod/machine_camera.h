/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Code based on circuitpython camera API by Jeff Epler
 * Copyright (c) 2022 Jeff Epler for Adafruit Industries
 * Adaptation to MicroPython by Christopher Nadler
 * Copyright (c) 2024 Christopher Nadler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/enum.h"
#include "py/obj.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/modmachine.h"

#if MICROPY_PY_MACHINE_CAMERA

typedef struct _camera_obj_t {
    mp_obj_base_t       base;
    mp_camera_config_t  config;
    bool                initialized;
} mp_camera_obj_t;

// TODO: replace with MP pin types
typedef struct {
    int pin_d0;
    int pin_d1;
    int pin_d2;
    int pin_d3;
    int pin_d4;
    int pin_d5;
    int pin_d6;
    int pin_d7;
    int pin_vsync;
    int pin_href;
    int pin_pclk;
    int pin_pwdn;
    int pin_reset;
    int pin_xclk;
    int pin_sscb_sda;
    int pin_sscb_scl;
    int xclk_freq_hz;
    int fb_count;
    int jpeg_quality;
    mp_camera_pixformat_t pixel_format;
    mp_camera_framesize_t frame_size;
    mp_camera_fb_location_t fb_location;
    mp_camera_grab_mode_t grab_mode;
} mp_camera_config_t;

//TODO: decide if reorder or not
typedef enum {
    PIXFORMAT_RGB565,    // 2BPP/RGB565
    PIXFORMAT_YUV422,    // 2BPP/YUV422
    PIXFORMAT_YUV420,    // 1.5BPP/YUV420
    PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
    PIXFORMAT_JPEG,      // JPEG/COMPRESSED
    PIXFORMAT_RGB888,    // 3BPP/RGB888
    PIXFORMAT_RAW,       // RAW
    PIXFORMAT_RGB444,    // 3BP2P/RGB444
    PIXFORMAT_RGB555,    // 3BP2P/RGB555
} mp_camera_pixformat_t;

typedef enum {
    FRAMESIZE_96X96,    // 96x96
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_240X240,  // 240x240
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 400x296
    FRAMESIZE_HVGA,     // 480x320
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    // 3MP Sensors
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_P_HD,     //  720x1280
    FRAMESIZE_P_3MP,    //  864x1536
    FRAMESIZE_QXGA,     // 2048x1536
    // 5MP Sensors
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_P_FHD,    // 1080x1920
    FRAMESIZE_QSXGA,    // 2560x1920
} mp_camera_framesize_t;

typedef enum {
    GAINCEILING_2X,
    GAINCEILING_4X,
    GAINCEILING_8X,
    GAINCEILING_16X,
    GAINCEILING_32X,
    GAINCEILING_64X,
    GAINCEILING_128X,
} mp_camera_gainceiling_t;
typedef enum {
    CAMERA_FB_IN_PSRAM,         /*!< Frame buffer is placed in external PSRAM */
    CAMERA_FB_IN_DRAM,           /*!< Frame buffer is placed in internal DRAM */
} mp_camera_fb_location_t;

typedef enum {
    CAMERA_GRAB_WHEN_EMPTY,         /*!< Fills buffers when they are empty. Less resources but first 'fb_count' frames might be old */
    CAMERA_GRAB_LATEST,              /*!< Except when 1 frame buffer is used, queue will always contain the last 'fb_count' frames */
} mp_camera_grab_mode_t;

// TODO: replace with actual camera object and type
extern const mp_obj_type_t espcamera_camera_type;
typedef struct espcamera_camera_obj mp_camera_obj_t;

// TODO: define constructor and replace with MP-Types
extern void machine_hw_camera_construct(
    mp_camera_obj_t *self,
    uint8_t data_pins[8],
    const mcu_pin_obj_t *external_clock_pin,
    const mcu_pin_obj_t *pixel_clock_pin,
    const mcu_pin_obj_t *vsync_pin,
    const mcu_pin_obj_t *href_pin,
    const mcu_pin_obj_t *powerdown_pin,
    const mcu_pin_obj_t *reset_pin,
    busio_i2c_obj_t *i2c,
    mp_int_t external_clock_frequency,
    pixformat_t pixel_format,
    framesize_t frame_size,
    mp_int_t jpeg_quality,
    mp_int_t framebuffer_count,
    camera_grab_mode_t grab_mode);

extern void machine_hw_camera_init(mp_camera_obj_t *self);
extern void machine_hw_camera_deinit(mp_camera_obj_t *self);
extern void machine_hw_camera_reconfigure(mp_camera_obj_t *self, framesize_t frame_size, pixformat_t pixel_format, camera_grab_mode_t grab_mode, mp_int_t framebuffer_count);

//TODO: decide function signature
extern camera_fb_t *machine_hw_camera_capture(mp_camera_obj_t *self, int timeout_ms);

#define DECLARE_SENSOR_GETSET(type, name, field_name, setter_function_name) \
    DECLARE_SENSOR_GET(type, name, field_name, setter_function_name) \
    DECLARE_SENSOR_SET(type, name, setter_function_name)

#define DECLARE_SENSOR_STATUS_GETSET(type, name, status_field_name, setter_function_name) \
    DECLARE_SENSOR_GETSET(type, name, status.status_field_name, setter_function_name)

#define DECLARE_SENSOR_STATUS_GET(type, name, status_field_name, setter_function_name) \
    DECLARE_SENSOR_GET(type, name, status.status_field_name, setter_function_name)

#define DECLARE_SENSOR_GET(type, name, status_field_name, setter_function_name) \
    extern type machine_hw_camera_get_##name(mp_camera_obj_t * self);

#define DECLARE_SENSOR_SET(type, name, setter_function_name) \
    extern void machine_hw_camera_set_##name(mp_camera_obj_t * self, type value);

DECLARE_SENSOR_GET(mp_camera_pixformat_t, pixel_format, pixformat, set_pixformat)
DECLARE_SENSOR_STATUS_GET(mp_camera_framesize_t, frame_size, framesize, set_framesize)
DECLARE_SENSOR_STATUS_GETSET(int, contrast, contrast, set_contrast);
DECLARE_SENSOR_STATUS_GETSET(int, brightness, brightness, set_brightness);
DECLARE_SENSOR_STATUS_GETSET(int, saturation, saturation, set_saturation);
DECLARE_SENSOR_STATUS_GETSET(int, sharpness, sharpness, set_sharpness);
DECLARE_SENSOR_STATUS_GETSET(int, denoise, denoise, set_denoise);
DECLARE_SENSOR_STATUS_GETSET(mp_camera_gainceiling_t, gainceiling, gainceiling, set_gainceiling);
DECLARE_SENSOR_STATUS_GETSET(int, quality, quality, set_quality);
DECLARE_SENSOR_STATUS_GETSET(bool, colorbar, colorbar, set_colorbar);
DECLARE_SENSOR_STATUS_GETSET(bool, whitebal, whitebal, set_whitebal);
DECLARE_SENSOR_STATUS_GETSET(bool, gain_ctrl, gain_ctrl, set_gain_ctrl);
DECLARE_SENSOR_STATUS_GETSET(bool, exposure_ctrl, exposure_ctrl, set_exposure_ctrl);
DECLARE_SENSOR_STATUS_GETSET(bool, hmirror, hmirror, set_hmirror);
DECLARE_SENSOR_STATUS_GETSET(bool, vflip, vflip, set_vflip);
DECLARE_SENSOR_STATUS_GETSET(bool, aec2, aec2, set_aec2);
DECLARE_SENSOR_STATUS_GETSET(bool, awb_gain, awb_gain, set_awb_gain);
DECLARE_SENSOR_STATUS_GETSET(int, agc_gain, agc_gain, set_agc_gain);
DECLARE_SENSOR_STATUS_GETSET(int, aec_value, aec_value, set_aec_value);
DECLARE_SENSOR_STATUS_GETSET(int, special_effect, special_effect, set_special_effect);
DECLARE_SENSOR_STATUS_GETSET(int, wb_mode, wb_mode, set_wb_mode);
DECLARE_SENSOR_STATUS_GETSET(int, ae_level, ae_level, set_ae_level);
DECLARE_SENSOR_STATUS_GETSET(bool, dcw, dcw, set_dcw);
DECLARE_SENSOR_STATUS_GETSET(bool, bpc, bpc, set_bpc);
DECLARE_SENSOR_STATUS_GETSET(bool, wpc, wpc, set_wpc);
DECLARE_SENSOR_STATUS_GETSET(bool, raw_gma, raw_gma, set_raw_gma);
DECLARE_SENSOR_STATUS_GETSET(bool, lenc, lenc, set_lenc);

// From settings
extern camera_grab_mode_t machine_hw_camera_get_grab_mode(mp_camera_obj_t *self);
extern int machine_hw_camera_get_framebuffer_count(mp_camera_obj_t *self);

// From camera_sensor_info_t
extern int machine_hw_camera_get_address(mp_camera_obj_t *self);
extern const char *machine_hw_camera_get_sensor_name(mp_camera_obj_t *self);
extern const bool machine_hw_camera_get_supports_jpeg(mp_camera_obj_t *self);
extern framesize_t machine_hw_camera_get_max_frame_size(mp_camera_obj_t *self);
extern int machine_hw_camera_get_width(mp_camera_obj_t *self);
extern int machine_hw_camera_get_height(mp_camera_obj_t *self);

#endif // MICROPY_PY_MACHINE_CAMERA