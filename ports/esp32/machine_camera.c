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
#include "py/mperrno.h"
#include "extmod/modmachine.h" //Needs to add machine_camera_type and maybe other things, or do we handle everything here?
#include "py/mphal.h" //Maybe we can add here MICROPY_PY_MACHINE_CAMERA (0), otherwise not needed

#if (CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3) && MICROPY_PY_MACHINE_CAMERA
#include "esp_camera.h" //maybe driver/esp_camera.h, but don't know yet where will the driver be located
#include "esp_err.h"
#include "machine_camera.h" //don't think I will need a searate header

#if !CONFIG_SPIRAM
#error espcamera only works on boards configured with spiram
#endif

typedef struct _camera_obj_t {
    mp_obj_base_t       base;
    camera_config_t     config;         //Type defined in esp32_camera driver
    bool                initialized;
} mp_camera_obj_t;

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

void machine_hw_camera_init(mp_camera_obj_t *self) {
    camera_config_t temp_config = self->camera_config;
    temp_config.frame_size = FRAMESIZE_QVGA;       //use values supported by all cameras
    temp_config.pixel_format = PIXFORMAT_RGB565;    //use values supported by all cameras
    esp_err_t err = esp_camera_init(&temp_config);
    raise_micropython_error_from_esp_err(err);
    self->initialized = true;
    machine_hw_camera_reconfigure(&self);
}

void machine_hw_camera_deinit(mp_camera_obj_t *self) {
    if (self->initialized) {
        esp_err_t err = esp_camera_deinit();
        raise_micropython_error_from_esp_err(err);
        self->initialized = false;
    }
}

mp_obj_t machine_hw_camera_capture(mp_camera_obj_t *self) {
    // TODO: check if self is intialized
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        mp_raise_OSError_msg_varg(MP_EIO, "Failed to capture image: Camera returned no data");
    }
    mp_obj_t image = mp_obj_new_bytes(fb->buf, fb->len);
    esp_camera_fb_return(fb); //return memory
    return image;
}

void machine_hw_camera_reconfigure(mp_camera_obj_t *self) {
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

        esp_err_t err = esp_camera_deinit();
        // TODO: set init flag to false if error
        raise_micropython_error_from_esp_err(err);
        esp_err_t err = esp_camera_init(&camera->config);
        // TODO: set init flag to false if error
        raise_micropython_error_from_esp_err(err);
    }
}

// Helper functions to get camera information
pixformat_t machine_hw_camera_get_pixel_format(mp_camera_obj_t *self) {
    return self->camera_config.pixel_format;
}

framesize_t machine_hw_camera_get_frame_size(mp_camera_obj_t *self) {
    return self->camera_config.frame_size;
}

const camera_grab_mode_t machine_hw_camera_get_grab_mode(mp_camera_obj_t *self) {
    return self->camera_config.grab_mode;
}

const int machine_hw_camera_get_framebuffer_count(mp_camera_obj_t *self) {
    return self->camera_config.fb_count;
}

//TODO: add init flag to helper methods
const char *machine_hw_camera_get_sensor_name(mp_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->name;
}

const bool machine_hw_camera_get_supports_jpeg(mp_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->support_jpeg;
}

const framesize_t machine_hw_camera_get_max_frame_size(mp_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->max_size;
}

const int machine_hw_camera_get_address(mp_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    camera_sensor_info_t *sensor_info = esp_camera_sensor_get_info(&sensor->id);
    return sensor_info->sccb_addr;
}

const int machine_hw_camera_get_width(mp_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    framesize_t framesize = sensor->status.framesize;
    return resolution[framesize].width;
}

const int machine_hw_camera_get_height(mp_camera_obj_t *self) {
    sensor_t *sensor = esp_camera_sensor_get();
    framesize_t framesize = sensor->status.framesize;
    return resolution[framesize].height;
}

#endif // CONFIG_ESP32_CAMERA_ENABLED