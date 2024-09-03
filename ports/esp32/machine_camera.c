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

#include "py/runtime.h"
#include "py/mperrno.h"
#include "extmod/modmachine.h" //Needs to add machine_camera_type and maybe other things, or do we handle everything here?
#include "py/mphal.h" //Maybe we can add here MICROPY_PY_MACHINE_CAMERA (0), otherwise not needed

#if (CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3) && MICROPY_PY_MACHINE_CAMERA
#include "esp_camera.h" //maybe driver/esp_camera.h, but don't know yet where will the driver be located in esp-idf
#include "esp_err.h"
#include "machine_camera.h" //don't think I will need a searate header

#if !CONFIG_SPIRAM
#error espcamera only works on boards configured with spiram
#endif

typedef struct _camera_obj_t {
    mp_obj_base_t       base;
    camera_config_t     config;         //Type defined in esp32_camera driver
    bool                initialized;
} machine_hw_camera_obj_t; //Maybe in separate header file

void raise_micropython_error_from_esp_err(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return;

        case ESP_ERR_NO_MEM:
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("ESP_ERR_NO_MEM: Out of memory"));
            break;

        case ESP_ERR_INVALID_ARG:
            mp_raise_ValueError(MP_ERROR_TEXT("ESP_ERR_INVALID_ARG: Invalid argument"));
            break;

        case ESP_ERR_INVALID_STATE:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("ESP_ERR_INVALID_STATE: Invalid state"));
            break;

        case ESP_ERR_NOT_FOUND:
            mp_raise_OSError(MP_ENOENT);
            break;

        case ESP_ERR_NOT_SUPPORTED:
            mp_raise_NotImplementedError(MP_ERROR_TEXT("ESP_ERR_NOT_SUPPORTED: Operation not supported"));
            break;

        case ESP_ERR_TIMEOUT:
            mp_raise_OSError(MP_ETIMEDOUT);
            break;

        default:
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("ESP_ERR: Unknown error 0x%04x"), err);
            break;
    }
}

void machine_hw_camera_init(machine_hw_camera_obj_t *self) {
    if (self->initialized) {
        return;
    }
    camera_config_t temp_config = self->camera_config;
    temp_config.frame_size = FRAMESIZE_QVGA;       //use values supported by all cameras
    temp_config.pixel_format = PIXFORMAT_RGB565;    //use values supported by all cameras
    esp_err_t err = esp_camera_init(&temp_config);
    raise_micropython_error_from_esp_err(err);
    self->initialized = true;
    machine_hw_camera_reconfigure(&self);
}

void machine_hw_camera_deinit(machine_hw_camera_obj_t *self) {
    if (self->initialized) {
        esp_err_t err = esp_camera_deinit();
        raise_micropython_error_from_esp_err(err);
        self->initialized = false;
    }
}

mp_obj_t machine_hw_camera_capture(machine_hw_camera_obj_t *self) {
    if (!self->initialized) {
        mp_raise_OSError_msg_varg(MP_EIO, "Failed to capture image: Camera not initialized");
    }
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        mp_raise_OSError_msg_varg(MP_EIO, "Failed to capture image: Camera returned no data");
    }
    mp_obj_t image = mp_obj_new_bytes(fb->buf, fb->len);
    esp_camera_fb_return(fb); //return memory
    return image;
}

void machine_hw_camera_reconfigure(machine_hw_camera_obj_t *self) {
    if (self->initialized) {
        sensor_t *sensor = esp_camera_sensor_get();
        camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);

        if (PIXFORMAT_JPEG == self->camera_config.pixel_format && (!sensor_info->support_jpeg)) {
            mp_raise_NotImplementedError(MP_ERROR_TEXT("Sensor does not support JPEG"));
        }

        if (self->camera_config.frame_size > sensor_info->max_size) {
            mp_warning(NULL, "Frame size will be scaled down to maximal frame size supported by the camera sensor");
            frame_size = sensor_info->max_size;
        }
        
        raise_micropython_error_from_esp_err(esp_camera_deinit());

        esp_err_t err = esp_camera_init(&camera->config);
        if (err != ESP_OK) {
            self->initialized = false;
            raise_micropython_error_from_esp_err(err);
        }
    }
}

// Helper functions to get and set camera and sensor information
// TODO: add init flag to helper methods

#define SENSOR_STATUS_GETSET(type, name, status_field_name, setter_function_name) \
    SENSOR_GETSET(type, name, status.status_field_name, setter_function_name)

#define SENSOR_GETSET(type, name, field_name, setter_function_name) \
    SENSOR_GET(type, name, field_name, setter_function_name) \
    SENSOR_SET(type, name, setter_function_name)

#define SENSOR_GET(type, name, status_field_name, getter_function_name) \
    type machine_hw_camera_get_##name(machine_hw_camera_obj_t * self) { \
        sensor_t *sensor = esp_camera_sensor_get(); \
        if (!sensor->getter_function_name) { \
            mp_raise_AttributeError(MP_ERROR_TEXT("no such attribute")); \
        } \
        return sensor->status_field_name; \
    }

#define SENSOR_SET(type, name, setter_function_name) \
    void machine_hw_camera_set_##name(machine_hw_camera_obj_t * self, type value) { \
        sensor_t *sensor = esp_camera_sensor_get(); \
        if (!sensor->setter_function_name) { \
            mp_raise_AttributeError(MP_ERROR_TEXT("no such attribute")); \
        } \
        if (sensor->setter_function_name(sensor, value) < 0) { \
            mp_raise_ValueError(MP_ERROR_TEXT("invalid setting")); \
        } \
    }

SENSOR_STATUS_GETSET(int, contrast, contrast, set_contrast);
SENSOR_STATUS_GETSET(int, brightness, brightness, set_brightness);
SENSOR_STATUS_GETSET(int, saturation, saturation, set_saturation);
SENSOR_STATUS_GETSET(int, sharpness, sharpness, set_sharpness);
SENSOR_STATUS_GETSET(int, denoise, denoise, set_denoise);
SENSOR_STATUS_GETSET(gainceiling_t, gainceiling, gainceiling, set_gainceiling);
SENSOR_STATUS_GETSET(int, quality, quality, set_quality);
SENSOR_STATUS_GETSET(bool, colorbar, colorbar, set_colorbar);
SENSOR_STATUS_GETSET(bool, whitebal, awb, set_whitebal);
SENSOR_STATUS_GETSET(bool, gain_ctrl, agc, set_gain_ctrl);
SENSOR_STATUS_GETSET(bool, exposure_ctrl, aec, set_exposure_ctrl);
SENSOR_STATUS_GETSET(bool, hmirror, hmirror, set_hmirror);
SENSOR_STATUS_GETSET(bool, vflip, vflip, set_vflip);
SENSOR_STATUS_GETSET(bool, aec2, aec2, set_aec2);
SENSOR_STATUS_GETSET(bool, awb_gain, awb_gain, set_awb_gain);
SENSOR_STATUS_GETSET(int, agc_gain, agc_gain, set_agc_gain);
SENSOR_STATUS_GETSET(int, aec_value, aec_value, set_aec_value);
SENSOR_STATUS_GETSET(int, special_effect, special_effect, set_special_effect);
SENSOR_STATUS_GETSET(int, wb_mode, wb_mode, set_wb_mode);
SENSOR_STATUS_GETSET(int, ae_level, ae_level, set_ae_level);
SENSOR_STATUS_GETSET(bool, dcw, dcw, set_dcw);
SENSOR_STATUS_GETSET(bool, bpc, bpc, set_bpc);
SENSOR_STATUS_GETSET(bool, wpc, wpc, set_wpc);
SENSOR_STATUS_GETSET(bool, raw_gma, raw_gma, set_raw_gma);
SENSOR_STATUS_GETSET(bool, lenc, lenc, set_lenc);


pixformat_t machine_hw_camera_get_pixel_format(machine_hw_camera_obj_t *self) {
    return self->camera_config.pixel_format;
}

framesize_t machine_hw_camera_get_frame_size(machine_hw_camera_obj_t *self) {
    return self->camera_config.frame_size;
}

const camera_grab_mode_t machine_hw_camera_get_grab_mode(machine_hw_camera_obj_t *self) {
    return self->camera_config.grab_mode;
}

const int machine_hw_camera_get_framebuffer_count(machine_hw_camera_obj_t *self) {
    return self->camera_config.fb_count;
}

const char *machine_hw_camera_get_sensor_name(machine_hw_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->name;
}

const bool machine_hw_camera_get_supports_jpeg(machine_hw_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->support_jpeg;
}

const framesize_t machine_hw_camera_get_max_frame_size(machine_hw_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->max_size;
}

const int machine_hw_camera_get_address(machine_hw_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->sccb_addr;
}

// TODO: Need to take a look at resolution
// const int machine_hw_camera_get_width(machine_hw_camera_obj_t *self) {
//     sensor_t *sensor = esp_camera_sensor_get();
//     framesize_t framesize = sensor->status.framesize;
//     return resolution[framesize].width;
// }

// const int machine_hw_camera_get_height(machine_hw_camera_obj_t *self) {
//     sensor_t *sensor = esp_camera_sensor_get();
//     framesize_t framesize = sensor->status.framesize;
//     return resolution[framesize].height;
// }

/******************************************************************************/
// MicroPython bindings for machine API

static void machine_hw_camera_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_hw_camera_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // TODO
    mp_printf(print, "",);
}

mp_obj_t machine_hw_camera_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // TODO
    return MP_OBJ_FROM_PTR(self);
}

// locals_dict, &mp_machine_camera_locals_dict maybe needed, also, MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS instead of MP_TYPE_FLAG_NONE possible
MP_DEFINE_CONST_OBJ_TYPE(
    machine_camera_type,
    MP_QSTR_CAMERA,
    MP_TYPE_FLAG_NONE,
    make_new, machine_hw_camera_make_new,
    print, machine_hw_camera_print
    );

#endif // ESP32 camera supported