/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

esp_err_t display_init(void)
{
    // lvgl 画一个输入框
    lv_obj_t *lable = lv_label_create(lv_scr_act());
    lv_label_set_text(lable, "Hello world!");
    lv_obj_set_size(lable, 200, 50);
    lv_obj_align(lable, LV_ALIGN_CENTER, 0, 0);

    return ESP_OK;
}