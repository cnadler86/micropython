/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "extmod/modmachine.h"

#if MICROPY_PY_MACHINE_CAMERA
#include "machine_camera.h"

// static mp_obj_t camera_init(mp_map_t *kw_args) {
//     camera_obj_t *camera = m_new_obj_with_finaliser(camera_obj_t);
//     camera->base.type = &camera_type;  // Typ festlegen
    
//     enum {
//         ARG_format,
//         ARG_framesize,
//         ARG_quality,
//         ARG_d0,
//         ARG_d1,
//         ARG_d2,
//         ARG_d3,
//         ARG_d4,
//         ARG_d5,
//         ARG_d6,
//         ARG_d7,
//         ARG_VSYNC,
//         ARG_HREF,
//         ARG_PCLK,
//         ARG_PWDN,
//         ARG_RESET,
//         ARG_XCLK,
//         ARG_SIOD,
//         ARG_SIOC,
//         ARG_FREQ,
//         ARG_FBSIZE,
//         ARG_FBLOC,
//         ARG_GRABMODE
//     };

//     static const mp_arg_t allowed_args[] = {
//         { MP_QSTR_format,       MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = PIXFORMAT_JPEG} },
//         { MP_QSTR_framesize,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = FRAMESIZE_HVGA} },
//         { MP_QSTR_quality,      MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 12} },
//         { MP_QSTR_d0,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D0} },
//         { MP_QSTR_d1,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D1} },
//         { MP_QSTR_d2,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D2} },
//         { MP_QSTR_d3,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D3} },
//         { MP_QSTR_d4,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D4} },
//         { MP_QSTR_d5,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D5} },
//         { MP_QSTR_d6,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D6} },
//         { MP_QSTR_d7,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D7} },
//         { MP_QSTR_vsync,        MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_VSYNC} },
//         { MP_QSTR_href,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_HREF} },
//         { MP_QSTR_pclk,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_PCLK} },
//         { MP_QSTR_pwdn,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_PWDN} },
//         { MP_QSTR_reset,        MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_RESET} },
//         { MP_QSTR_xclk,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_XCLK} },
//         { MP_QSTR_siod,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_SIOD} },
//         { MP_QSTR_sioc,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_SIOC} },
//         { MP_QSTR_xclk_freq,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = XCLK_FREQ_20MHz} },
//         { MP_QSTR_fb_size,      MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 2} },
//         { MP_QSTR_fb_location,  MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAMERA_FB_IN_PSRAM} },
//         { MP_QSTR_grab_mode,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAMERA_GRAB_LATEST} },
//     };
//     mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
//     mp_arg_parse_all(0, 0, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

//     int32_t xclk_freq = args[ARG_FREQ].u_int;
//     if ((xclk_freq < XCLK_FREQ_10MHz) ||
//         (xclk_freq > XCLK_FREQ_20MHz)) {
//         mp_raise_ValueError(MP_ERROR_TEXT("xclk frequency is not in valid range"));
//     }

//     // configure camera based on arguments
//     camera->config.pixel_format = args[ARG_format].u_int;   //YUV422,GRAYSCALE,RGB565,JPEG
//     camera->config.frame_size = args[ARG_framesize].u_int; //QQVGA-QXGA Do not use sizes above QVGA when not JPEG
//     camera->config.jpeg_quality = args[ARG_quality].u_int;  //0-63 lower number means higher quality
//     camera->config.pin_d0 = args[ARG_d0].u_int;
//     camera->config.pin_d1 = args[ARG_d1].u_int;
//     camera->config.pin_d2 = args[ARG_d2].u_int;
//     camera->config.pin_d3 = args[ARG_d3].u_int;
//     camera->config.pin_d4 = args[ARG_d4].u_int;
//     camera->config.pin_d5 = args[ARG_d5].u_int;
//     camera->config.pin_d6 = args[ARG_d6].u_int;
//     camera->config.pin_d7 = args[ARG_d7].u_int;
//     camera->config.pin_vsync = args[ARG_VSYNC].u_int;
//     camera->config.pin_href = args[ARG_HREF].u_int;
//     camera->config.pin_pclk = args[ARG_PCLK].u_int;
//     camera->config.pin_pwdn = args[ARG_PWDN].u_int;
//     camera->config.pin_reset = args[ARG_RESET].u_int;
//     camera->config.pin_xclk = args[ARG_XCLK].u_int;
//     camera->config.pin_sscb_sda = args[ARG_SIOD].u_int;
//     camera->config.pin_sscb_scl = args[ARG_SIOC].u_int;
//     camera->config.xclk_freq_hz = args[ARG_FREQ].u_int;
//     camera->config.fb_count = args[ARG_FBSIZE].u_int;      //if more than one, i2s runs in continuous mode. Use only with JPEG
//     camera->config.fb_location = args[ARG_FBLOC].u_int;
//     camera->config.grab_mode = args[ARG_GRABMODE].u_int;

//     // defaul parameters
//     camera->config.ledc_timer = LEDC_TIMER_0;
//     camera->config.ledc_channel = LEDC_CHANNEL_0;
    
//     esp32cam_init_low(&camera->config)
// }
// static MP_DEFINE_CONST_FUN_OBJ_KW(camera_init_obj, 0, camera_init)

// static mp_obj_t camera_deinit(){
//     esp_err_t err = esp_camera_deinit();
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "Camera deinit failed");
//         return mp_const_false;
//     }
//     return mp_const_true;
// }
// static MP_DEFINE_CONST_FUN_OBJ_0(camera_deinit_obj, camera_deinit);

static mp_obj_t espcamera_camera_make_new_stub(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    (void)type;
    (void)n_args;
    (void)n_kw;
    (void)all_args;
    mp_camera_obj_t *self = m_new_obj_with_finaliser(mp_camera_obj_t);
    self->base.type = &mp_camera_obj_t;

    framesize_t frame_size = FRAMESIZE_HVGA;
    pixformat_t pixel_format = PIXFORMAT_JPEG;
    camera_grab_mode_t grab_mode = CAMERA_GRAB_LATEST;


    machine_hw_camera_construct(
        self,
        [15,17,18,16,14,12,11,48],
        10,
        13,
        38,
        47,
        -1,
        -1,
        40,
        39,
        20000000,
        pixel_format,
        frame_size,
        10,
        2,
        grab_mode);
        machine_hw_camera_init(self);
        (void)machine_hw_camera_capture(self, 1000);
    return MP_OBJ_FROM_PTR(self);
}

// static mp_obj_t espcamera_camera_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
//     enum { ARG_data_pins, ARG_pixel_clock_pin, ARG_vsync_pin, ARG_href_pin, ARG_i2c, ARG_external_clock_pin, ARG_external_clock_frequency, ARG_powerdown_pin, ARG_reset_pin, ARG_pixel_format, ARG_frame_size, ARG_jpeg_quality, ARG_framebuffer_count, ARG_grab_mode, NUM_ARGS };
//     static const mp_arg_t allowed_args[] = {
//         { MP_QSTR_data_pins, MP_ARG_OBJ | MP_ARG_KW_ONLY | MP_ARG_REQUIRED },
//         { MP_QSTR_pixel_clock_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY | MP_ARG_REQUIRED },
//         { MP_QSTR_vsync_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY | MP_ARG_REQUIRED },
//         { MP_QSTR_href_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY | MP_ARG_REQUIRED },
//         { MP_QSTR_sccb_sda_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY | MP_ARG_REQUIRED },
//         { MP_QSTR_sccb_scl_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY | MP_ARG_REQUIRED },
//         { MP_QSTR_external_clock_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY, { .u_obj = MP_ROM_NONE } },
//         { MP_QSTR_external_clock_frequency, MP_ARG_INT | MP_ARG_KW_ONLY, { .u_int = 20000000L } },
//         { MP_QSTR_powerdown_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY, { .u_obj = MP_ROM_NONE } },
//         { MP_QSTR_reset_pin, MP_ARG_OBJ | MP_ARG_KW_ONLY, { .u_obj = MP_ROM_NONE } },
//         { MP_QSTR_pixel_format, MP_ARG_OBJ | MP_ARG_KW_ONLY, { .u_obj = MP_ROM_PTR((void *)&pixel_format_RGB565_obj) } },
//         { MP_QSTR_frame_size, MP_ARG_OBJ | MP_ARG_KW_ONLY, { .u_obj = MP_ROM_PTR((void *)&frame_size_QQVGA_obj) } },
//         { MP_QSTR_jpeg_quality, MP_ARG_INT | MP_ARG_KW_ONLY, { .u_int = 15 } },
//         { MP_QSTR_framebuffer_count, MP_ARG_INT | MP_ARG_KW_ONLY, { .u_int = 2 } },
//         { MP_QSTR_grab_mode, MP_ARG_OBJ | MP_ARG_KW_ONLY, { .u_obj = MP_ROM_PTR((void *)&grab_mode_WHEN_EMPTY_obj) } },
//     };

//     mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
//     MP_STATIC_ASSERT(MP_ARRAY_SIZE(allowed_args) == NUM_ARGS);
//     mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

//     uint8_t data_pins[8];
//     uint8_t data_pin_count;
//     // validate_pins(MP_QSTR_data_pins, data_pins, MP_ARRAY_SIZE(data_pins), args[ARG_data_pins].u_obj, &data_pin_count);
//     mp_arg_validate_length(data_pin_count, 8, MP_QSTR_data_pins);

//     // const mcu_pin_obj_t *pixel_clock_pin =
//     //     validate_obj_is_free_pin(args[ARG_pixel_clock_pin].u_obj, MP_QSTR_pixel_clock_pin);
//     // const mcu_pin_obj_t *vsync_pin =
//     //     validate_obj_is_free_pin(args[ARG_vsync_pin].u_obj, MP_QSTR_vsync_pin);
//     // const mcu_pin_obj_t *href_pin =
//     //     validate_obj_is_free_pin(args[ARG_href_pin].u_obj, MP_QSTR_href_pin);
//     // busio_i2c_obj_t *i2c = MP_OBJ_TO_PTR(mp_arg_validate_type(args[ARG_i2c].u_obj, &busio_i2c_type, MP_QSTR_i2c));
//     // const mcu_pin_obj_t *external_clock_pin =
//     //     validate_obj_is_free_pin_or_none(args[ARG_external_clock_pin].u_obj, MP_QSTR_external_clock_pin);
//     // const mcu_pin_obj_t *powerdown_pin =
//     //     validate_obj_is_free_pin_or_none(args[ARG_powerdown_pin].u_obj, MP_QSTR_powerdown_pin);
//     // const mcu_pin_obj_t *reset_pin =
//     //     validate_obj_is_free_pin_or_none(args[ARG_reset_pin].u_obj, MP_QSTR_reset_pin);
//     const mp_int_t external_clock_frequency =
//         mp_arg_validate_int_range(args[ARG_external_clock_frequency].u_int, 0, 40000000, MP_QSTR_external_clock_frequency);

//     // camera_grab_mode_t grab_mode = validate_grab_mode(args[ARG_grab_mode].u_obj, MP_QSTR_grab_mode);
//     // framesize_t frame_size = validate_frame_size(args[ARG_frame_size].u_obj, MP_QSTR_frame_size);
//     // pixformat_t pixel_format = validate_pixel_format(args[ARG_pixel_format].u_obj, MP_QSTR_pixel_format);
//     mp_int_t jpeg_quality = mp_arg_validate_int_range(args[ARG_jpeg_quality].u_int, 2, 55, MP_QSTR_jpeg_quality);
//     mp_int_t framebuffer_count = mp_arg_validate_int_range(args[ARG_framebuffer_count].u_int, 1, 2, MP_QSTR_framebuffer_count);

//     mp_camera_obj_t *self = m_new_obj_with_finaliser(mp_camera_obj_t);
//     self->base.type = &mp_camera_type;
//     machine_hw_camera_construct(
//         self,
//         data_pins,
//         external_clock_pin,
//         pixel_clock_pin,
//         vsync_pin,
//         href_pin,
//         powerdown_pin,
//         reset_pin,
//         sccb_sda_pin,
//         sccb_scl_pin,
//         external_clock_frequency,
//         pixel_format,
//         frame_size,
//         jpeg_quality,
//         framebuffer_count,
//         grab_mode);
//     return MP_OBJ_FROM_PTR(self);
// }

static mp_obj_t camera_capture(size_t n_args, const mp_obj_t *args){
    //DONE
    mp_camera_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_float_t timeout = n_args < 2 ? MICROPY_FLOAT_CONST(0.25) : mp_obj_get_float(args[1]);
    return mp_obj_t machine_hw_camera_capture(self, (int)MICROPY_FLOAT_C_FUN(round)(timeout * 1000));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(camera_capture_obj, 1, 2, camera_capture);

// static mp_obj_t camera_obj___exit__(size_t n_args, const mp_obj_t *args) {
//     (void)n_args;
//     (void)machine_hw_camera_deinit(args[0]);
//     return mp_const_none;
// }
// static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(camera___exit___obj, 4, 4, camera_obj___exit__);

// static mp_obj_t camera_flip(mp_obj_t direction){
//     sensor_t * s = esp_camera_sensor_get();
//     if (!s) {
//         ESP_LOGE(TAG, "Flipping Failed");
//         return mp_const_false;
//       }
//     int dir = mp_obj_get_int(direction);
//     s->set_vflip(s, dir);
//     return mp_const_none;
// }
// static MP_DEFINE_CONST_FUN_OBJ_1(camera_flip_obj, camera_flip);

// static mp_obj_t camera_mirror(mp_obj_t direction){
//     sensor_t * s = esp_camera_sensor_get();
//     if (!s) {
//         ESP_LOGE(TAG, "Mirroring Failed");
//         return mp_const_false;
//       }
//     int dir = mp_obj_get_int(direction);
//     s->set_hmirror(s, dir);
//     return mp_const_none;
// }
// static MP_DEFINE_CONST_FUN_OBJ_1(camera_mirror_obj, camera_mirror);



static const mp_rom_map_elem_t espcamera_camera_locals_table[] = {
    // { MP_ROM_QSTR(MP_QSTR_reconfigure), MP_ROM_PTR(&camera_reconfigure_obj) },
    { MP_ROM_QSTR(MP_QSTR_capture), MP_ROM_PTR(&camera_capture_obj) },
    // { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&espcamera_camera___exit___obj) },
}

// static const mp_rom_map_elem_t camera_module_globals_table[] = {
//     { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_camera) },

//     { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&camera_init_obj) },
//     { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&camera_deinit_obj) },
//     { MP_ROM_QSTR(MP_QSTR_capture), MP_ROM_PTR(&camera_capture_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_flip), MP_ROM_PTR(&camera_flip_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_mirror), MP_ROM_PTR(&camera_mirror_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_framesize), MP_ROM_PTR(&camera_framesize_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_quality), MP_ROM_PTR(&camera_quality_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_contrast), MP_ROM_PTR(&camera_contrast_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_saturation), MP_ROM_PTR(&camera_saturation_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_brightness), MP_ROM_PTR(&camera_brightness_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_speffect), MP_ROM_PTR(&camera_speffect_obj) },
//     // { MP_ROM_QSTR(MP_QSTR_whitebalance), MP_ROM_PTR(&camera_whitebalance_obj) },

//     // Constants
//     { MP_ROM_QSTR(MP_QSTR_JPEG),            MP_ROM_INT(PIXFORMAT_JPEG) },
//     { MP_ROM_QSTR(MP_QSTR_YUV422),          MP_ROM_INT(PIXFORMAT_YUV422) },
//     { MP_ROM_QSTR(MP_QSTR_GRAYSCALE),       MP_ROM_INT(PIXFORMAT_GRAYSCALE) },
//     { MP_ROM_QSTR(MP_QSTR_RGB565),          MP_ROM_INT(PIXFORMAT_RGB565) },
    
//     { MP_ROM_QSTR(MP_QSTR_FRAME_96X96),     MP_ROM_INT(FRAMESIZE_96X96) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_QQVGA),     MP_ROM_INT(FRAMESIZE_QQVGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_QCIF),      MP_ROM_INT(FRAMESIZE_QCIF) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_HQVGA),     MP_ROM_INT(FRAMESIZE_HQVGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_240X240),   MP_ROM_INT(FRAMESIZE_240X240) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_QVGA),      MP_ROM_INT(FRAMESIZE_QVGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_CIF),       MP_ROM_INT(FRAMESIZE_CIF) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_HVGA),      MP_ROM_INT(FRAMESIZE_HVGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_VGA),       MP_ROM_INT(FRAMESIZE_VGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_SVGA),      MP_ROM_INT(FRAMESIZE_SVGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_XGA),       MP_ROM_INT(FRAMESIZE_XGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_HD),        MP_ROM_INT(FRAMESIZE_HD) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_SXGA),      MP_ROM_INT(FRAMESIZE_SXGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_UXGA),      MP_ROM_INT(FRAMESIZE_UXGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_FHD),       MP_ROM_INT(FRAMESIZE_FHD) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_P_HD),      MP_ROM_INT(FRAMESIZE_P_HD) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_P_3MP),     MP_ROM_INT(FRAMESIZE_P_3MP) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_QXGA),      MP_ROM_INT(FRAMESIZE_QXGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_QHD),       MP_ROM_INT(FRAMESIZE_QHD) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_WQXGA),     MP_ROM_INT(FRAMESIZE_WQXGA) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_P_FHD),     MP_ROM_INT(FRAMESIZE_P_FHD) },
//     { MP_ROM_QSTR(MP_QSTR_FRAME_QSXGA),     MP_ROM_INT(FRAMESIZE_QSXGA) },
//     { MP_ROM_QSTR(MP_QSTR_GRAB_WHEN_EMPTY), MP_ROM_INT(CAMERA_GRAB_WHEN_EMPTY) },
//     { MP_ROM_QSTR(MP_QSTR_GRAB_LATEST),     MP_ROM_INT(CAMERA_GRAB_LATEST) },

//     // { MP_ROM_QSTR(MP_QSTR_WB_NONE),         MP_ROM_INT(WB_NONE) },
//     // { MP_ROM_QSTR(MP_QSTR_WB_SUNNY),        MP_ROM_INT(WB_SUNNY) },
//     // { MP_ROM_QSTR(MP_QSTR_WB_CLOUDY),       MP_ROM_INT(WB_CLOUDY) },
//     // { MP_ROM_QSTR(MP_QSTR_WB_OFFICE),       MP_ROM_INT(WB_OFFICE) },
//     // { MP_ROM_QSTR(MP_QSTR_WB_HOME),         MP_ROM_INT(WB_HOME) },

//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_NONE),     MP_ROM_INT(EFFECT_NONE) },
//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_NEG),      MP_ROM_INT(EFFECT_NEG) },
//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_BW),       MP_ROM_INT(EFFECT_BW) },
//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_RED),      MP_ROM_INT(EFFECT_RED) },
//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_GREEN),    MP_ROM_INT(EFFECT_GREEN) },
//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_BLUE),     MP_ROM_INT(EFFECT_BLUE) },
//     // { MP_ROM_QSTR(MP_QSTR_EFFECT_RETRO),    MP_ROM_INT(EFFECT_RETRO) },

//     { MP_ROM_QSTR(MP_QSTR_XCLK_10MHz),      MP_ROM_INT(XCLK_FREQ_10MHz) },
//     { MP_ROM_QSTR(MP_QSTR_XCLK_20MHz),      MP_ROM_INT(XCLK_FREQ_20MHz) },

//     { MP_ROM_QSTR(MP_QSTR_DRAM),            MP_ROM_INT(CAMERA_FB_IN_DRAM) },
//     { MP_ROM_QSTR(MP_QSTR_PSRAM),           MP_ROM_INT(CAMERA_FB_IN_PSRAM) },

// };

// static MP_DEFINE_CONST_DICT(camera_module_globals, camera_module_globals_table);

static MP_DEFINE_CONST_DICT(espcamera_camera_locals_dict, espcamera_camera_locals_table);

MP_DEFINE_CONST_OBJ_TYPE(
    mp_camera_type,
    MP_QSTR_Camera,
    make_new, espcamera_camera_make_new_stub,
    locals_dict, &espcamera_camera_locals_dict
    );
const mp_obj_module_t espcamera_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&espcamera_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_espcamera, espcamera_module);
#endif // MICROPY_PY_MACHINE_CAMERA