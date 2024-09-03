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

#include "py/runtime.h"
#include "py/mphal.h"
#include "modesp32cam.h"

#if CONFIG_ESP32_CAMERA_ENABLED
#include "modesp32cam.h"

typedef struct _camera_obj_t {
    mp_obj_base_t       base;
    camera_config_t     config; //TODO: define camera_config_t where?
    bool                initialized;
} mp_camera_obj_t;

static mp_obj_t camera_init(mp_map_t *kw_args) {
    camera_obj_t *camera = m_new_obj_with_finaliser(camera_obj_t);
    camera->base.type = &camera_type;  // Typ festlegen
    
    enum {
        ARG_format,
        ARG_framesize,
        ARG_quality,
        ARG_d0,
        ARG_d1,
        ARG_d2,
        ARG_d3,
        ARG_d4,
        ARG_d5,
        ARG_d6,
        ARG_d7,
        ARG_VSYNC,
        ARG_HREF,
        ARG_PCLK,
        ARG_PWDN,
        ARG_RESET,
        ARG_XCLK,
        ARG_SIOD,
        ARG_SIOC,
        ARG_FREQ,
        ARG_FBSIZE,
        ARG_FBLOC,
        ARG_GRABMODE
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_format,       MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = PIXFORMAT_JPEG} },
        { MP_QSTR_framesize,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = FRAMESIZE_HVGA} },
        { MP_QSTR_quality,      MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 12} },
        { MP_QSTR_d0,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D0} },
        { MP_QSTR_d1,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D1} },
        { MP_QSTR_d2,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D2} },
        { MP_QSTR_d3,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D3} },
        { MP_QSTR_d4,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D4} },
        { MP_QSTR_d5,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D5} },
        { MP_QSTR_d6,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D6} },
        { MP_QSTR_d7,           MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_D7} },
        { MP_QSTR_vsync,        MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_VSYNC} },
        { MP_QSTR_href,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_HREF} },
        { MP_QSTR_pclk,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_PCLK} },
        { MP_QSTR_pwdn,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_PWDN} },
        { MP_QSTR_reset,        MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_RESET} },
        { MP_QSTR_xclk,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_XCLK} },
        { MP_QSTR_siod,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_SIOD} },
        { MP_QSTR_sioc,         MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAM_PIN_SIOC} },
        { MP_QSTR_xclk_freq,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = XCLK_FREQ_20MHz} },
        { MP_QSTR_fb_size,      MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = 2} },
        { MP_QSTR_fb_location,  MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAMERA_FB_IN_PSRAM} },
        { MP_QSTR_grab_mode,    MP_ARG_KW_ONLY  | MP_ARG_INT,   {.u_int = CAMERA_GRAB_LATEST} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(0, 0, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int32_t xclk_freq = args[ARG_FREQ].u_int;
    if ((xclk_freq < XCLK_FREQ_10MHz) ||
        (xclk_freq > XCLK_FREQ_20MHz)) {
        mp_raise_ValueError(MP_ERROR_TEXT("xclk frequency is not in valid range"));
    }

    // configure camera based on arguments
    camera->config.pixel_format = args[ARG_format].u_int;   //YUV422,GRAYSCALE,RGB565,JPEG
    camera->config.frame_size = args[ARG_framesize].u_int; //QQVGA-QXGA Do not use sizes above QVGA when not JPEG
    camera->config.jpeg_quality = args[ARG_quality].u_int;  //0-63 lower number means higher quality
    camera->config.pin_d0 = args[ARG_d0].u_int;
    camera->config.pin_d1 = args[ARG_d1].u_int;
    camera->config.pin_d2 = args[ARG_d2].u_int;
    camera->config.pin_d3 = args[ARG_d3].u_int;
    camera->config.pin_d4 = args[ARG_d4].u_int;
    camera->config.pin_d5 = args[ARG_d5].u_int;
    camera->config.pin_d6 = args[ARG_d6].u_int;
    camera->config.pin_d7 = args[ARG_d7].u_int;
    camera->config.pin_vsync = args[ARG_VSYNC].u_int;
    camera->config.pin_href = args[ARG_HREF].u_int;
    camera->config.pin_pclk = args[ARG_PCLK].u_int;
    camera->config.pin_pwdn = args[ARG_PWDN].u_int;
    camera->config.pin_reset = args[ARG_RESET].u_int;
    camera->config.pin_xclk = args[ARG_XCLK].u_int;
    camera->config.pin_sscb_sda = args[ARG_SIOD].u_int;
    camera->config.pin_sscb_scl = args[ARG_SIOC].u_int;
    camera->config.xclk_freq_hz = args[ARG_FREQ].u_int;
    camera->config.fb_count = args[ARG_FBSIZE].u_int;      //if more than one, i2s runs in continuous mode. Use only with JPEG
    camera->config.fb_location = args[ARG_FBLOC].u_int;
    camera->config.grab_mode = args[ARG_GRABMODE].u_int;

    // defaul parameters
    camera->config.ledc_timer = LEDC_TIMER_0;
    camera->config.ledc_channel = LEDC_CHANNEL_0;
    
    esp32cam_init_low(&camera->config)
}
static MP_DEFINE_CONST_FUN_OBJ_KW(camera_init_obj, 0, camera_init)

static mp_obj_t camera_deinit(){
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera deinit failed");
        return mp_const_false;
    }
    return mp_const_true;
}
static MP_DEFINE_CONST_FUN_OBJ_0(camera_deinit_obj, camera_deinit);

static mp_obj_t camera_capture(){
    camera_fb_t * fb = esp_camera_fb_get();     //acquire a frame
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return mp_const_false;
    }
    mp_obj_t image = mp_obj_new_bytes(fb->buf, fb->len);    //create mpy image
    esp_camera_fb_return(fb); //return memory
    return image;
}
static MP_DEFINE_CONST_FUN_OBJ_0(camera_capture_obj, camera_capture);

static mp_obj_t camera_flip(mp_obj_t direction){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Flipping Failed");
        return mp_const_false;
      }
    int dir = mp_obj_get_int(direction);
    s->set_vflip(s, dir);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(camera_flip_obj, camera_flip);

static mp_obj_t camera_mirror(mp_obj_t direction){
    sensor_t * s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Mirroring Failed");
        return mp_const_false;
      }
    int dir = mp_obj_get_int(direction);
    s->set_hmirror(s, dir);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(camera_mirror_obj, camera_mirror);



static const mp_rom_map_elem_t espcamera_camera_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&espcamera_camera_deinit_obj) },
}

static const mp_rom_map_elem_t camera_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_camera) },

    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&camera_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&camera_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_capture), MP_ROM_PTR(&camera_capture_obj) },
    { MP_ROM_QSTR(MP_QSTR_flip), MP_ROM_PTR(&camera_flip_obj) },
    { MP_ROM_QSTR(MP_QSTR_mirror), MP_ROM_PTR(&camera_mirror_obj) },
    // { MP_ROM_QSTR(MP_QSTR_framesize), MP_ROM_PTR(&camera_framesize_obj) },
    // { MP_ROM_QSTR(MP_QSTR_quality), MP_ROM_PTR(&camera_quality_obj) },
    // { MP_ROM_QSTR(MP_QSTR_contrast), MP_ROM_PTR(&camera_contrast_obj) },
    // { MP_ROM_QSTR(MP_QSTR_saturation), MP_ROM_PTR(&camera_saturation_obj) },
    // { MP_ROM_QSTR(MP_QSTR_brightness), MP_ROM_PTR(&camera_brightness_obj) },
    // { MP_ROM_QSTR(MP_QSTR_speffect), MP_ROM_PTR(&camera_speffect_obj) },
    // { MP_ROM_QSTR(MP_QSTR_whitebalance), MP_ROM_PTR(&camera_whitebalance_obj) },

    // Constants
    { MP_ROM_QSTR(MP_QSTR_JPEG),            MP_ROM_INT(PIXFORMAT_JPEG) },
    { MP_ROM_QSTR(MP_QSTR_YUV422),          MP_ROM_INT(PIXFORMAT_YUV422) },
    { MP_ROM_QSTR(MP_QSTR_GRAYSCALE),       MP_ROM_INT(PIXFORMAT_GRAYSCALE) },
    { MP_ROM_QSTR(MP_QSTR_RGB565),          MP_ROM_INT(PIXFORMAT_RGB565) },
    
    { MP_ROM_QSTR(MP_QSTR_FRAME_96X96),     MP_ROM_INT(FRAMESIZE_96X96) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_QQVGA),     MP_ROM_INT(FRAMESIZE_QQVGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_QCIF),      MP_ROM_INT(FRAMESIZE_QCIF) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_HQVGA),     MP_ROM_INT(FRAMESIZE_HQVGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_240X240),   MP_ROM_INT(FRAMESIZE_240X240) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_QVGA),      MP_ROM_INT(FRAMESIZE_QVGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_CIF),       MP_ROM_INT(FRAMESIZE_CIF) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_HVGA),      MP_ROM_INT(FRAMESIZE_HVGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_VGA),       MP_ROM_INT(FRAMESIZE_VGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_SVGA),      MP_ROM_INT(FRAMESIZE_SVGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_XGA),       MP_ROM_INT(FRAMESIZE_XGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_HD),        MP_ROM_INT(FRAMESIZE_HD) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_SXGA),      MP_ROM_INT(FRAMESIZE_SXGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_UXGA),      MP_ROM_INT(FRAMESIZE_UXGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_FHD),       MP_ROM_INT(FRAMESIZE_FHD) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_P_HD),      MP_ROM_INT(FRAMESIZE_P_HD) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_P_3MP),     MP_ROM_INT(FRAMESIZE_P_3MP) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_QXGA),      MP_ROM_INT(FRAMESIZE_QXGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_QHD),       MP_ROM_INT(FRAMESIZE_QHD) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_WQXGA),     MP_ROM_INT(FRAMESIZE_WQXGA) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_P_FHD),     MP_ROM_INT(FRAMESIZE_P_FHD) },
    { MP_ROM_QSTR(MP_QSTR_FRAME_QSXGA),     MP_ROM_INT(FRAMESIZE_QSXGA) },
    { MP_ROM_QSTR(MP_QSTR_GRAB_WHEN_EMPTY), MP_ROM_INT(CAMERA_GRAB_WHEN_EMPTY) },
    { MP_ROM_QSTR(MP_QSTR_GRAB_LATEST),     MP_ROM_INT(CAMERA_GRAB_LATEST) },

    // { MP_ROM_QSTR(MP_QSTR_WB_NONE),         MP_ROM_INT(WB_NONE) },
    // { MP_ROM_QSTR(MP_QSTR_WB_SUNNY),        MP_ROM_INT(WB_SUNNY) },
    // { MP_ROM_QSTR(MP_QSTR_WB_CLOUDY),       MP_ROM_INT(WB_CLOUDY) },
    // { MP_ROM_QSTR(MP_QSTR_WB_OFFICE),       MP_ROM_INT(WB_OFFICE) },
    // { MP_ROM_QSTR(MP_QSTR_WB_HOME),         MP_ROM_INT(WB_HOME) },

    // { MP_ROM_QSTR(MP_QSTR_EFFECT_NONE),     MP_ROM_INT(EFFECT_NONE) },
    // { MP_ROM_QSTR(MP_QSTR_EFFECT_NEG),      MP_ROM_INT(EFFECT_NEG) },
    // { MP_ROM_QSTR(MP_QSTR_EFFECT_BW),       MP_ROM_INT(EFFECT_BW) },
    // { MP_ROM_QSTR(MP_QSTR_EFFECT_RED),      MP_ROM_INT(EFFECT_RED) },
    // { MP_ROM_QSTR(MP_QSTR_EFFECT_GREEN),    MP_ROM_INT(EFFECT_GREEN) },
    // { MP_ROM_QSTR(MP_QSTR_EFFECT_BLUE),     MP_ROM_INT(EFFECT_BLUE) },
    // { MP_ROM_QSTR(MP_QSTR_EFFECT_RETRO),    MP_ROM_INT(EFFECT_RETRO) },

    { MP_ROM_QSTR(MP_QSTR_XCLK_10MHz),      MP_ROM_INT(XCLK_FREQ_10MHz) },
    { MP_ROM_QSTR(MP_QSTR_XCLK_20MHz),      MP_ROM_INT(XCLK_FREQ_20MHz) },

    { MP_ROM_QSTR(MP_QSTR_DRAM),            MP_ROM_INT(CAMERA_FB_IN_DRAM) },
    { MP_ROM_QSTR(MP_QSTR_PSRAM),           MP_ROM_INT(CAMERA_FB_IN_PSRAM) },

};

static MP_DEFINE_CONST_DICT(camera_module_globals, camera_module_globals_table);

const mp_obj_module_t mp_module_esp32_camera = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&camera_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_camera, mp_module_esp32_camera);

#endif // CONFIG_ESP32_CAMERA_ENABLED