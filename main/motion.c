/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "motion.h"
#include "max30102.h"
#include "ui.h"
#include "app_sr_tts.h"

#define BAD_DRIVER_TIME 3000 * 1000
#define DANGER_DRIVER_TIME 9000 * 1000

extern QueueHandle_t max30102_queue;

static void motion_task(void *arg)
{
    uint32_t time_remaining = 0;
    uint32_t last_time = 0;
    max30102_data_t max30102_data;
    uint32_t driver_time = 0;
    uint32_t voice_cnt = 0;
    char dirver_time_str[20];
    uint32_t heart_cnt = 0;
    uint32_t o2_cnt = 0;
    while (1)
    {
        uint32_t current_time = esp_timer_get_time();
        driver_time = current_time;
        xQueueReceive(max30102_queue, &max30102_data, 100 / portTICK_PERIOD_MS);
        if (max30102_data.hand_detected) {
            time_remaining = 0;
        } else {
            last_time = current_time;
            current_time = esp_timer_get_time();
            time_remaining += current_time - last_time;
        }

        if (time_remaining > DANGER_DRIVER_TIME) {
            ui_change_mode(DANGER_MODE);
            if (voice_cnt++ % 300 == 0) {
                lv_label_set_text(ui_LabelTouch, "Off the wheel");
                app_tts_play("请握住方向盘");
            }
        } else if (time_remaining > BAD_DRIVER_TIME) {
            ui_change_mode(BAD_MODE);
            if (voice_cnt++ % 300 == 0) {
                lv_label_set_text(ui_LabelTouch, "Off the wheel");
                app_tts_play("请注意驾驶安全");
            }
        } else {
            ui_change_mode(SAFE_MODE);
            lv_label_set_text(ui_LabelTouch, "On the wheel");
        }

        sprintf(dirver_time_str, "time %02ld:%02ld:%02ld", driver_time / 1000000 / 3600, driver_time / 1000000 % 3600 / 60, driver_time / 1000000 % 60);
        lv_label_set_text(ui_LabelTime, dirver_time_str);

        if (driver_time > 120000 * 1000) {
            ui_change_mode(DANGER_MODE);
            if (voice_cnt++ % 1000 == 0) {
                app_tts_play("请及时休息，您已经连续驾驶超过四小时");
            }
        } else if ( driver_time > 60000 * 1000) {
            ui_change_mode(BAD_MODE);
            if (voice_cnt++ % 1000 == 0) {
                app_tts_play("请注意休息，您已经连续驾驶超过两小时");
            }
        }

        if ( max30102_data.hand_detected ) {
            if ( (max30102_data.heart_rate > 160 || max30102_data.heart_rate < 40) && max30102_data.heart_rate != 0 ) {
                if (heart_cnt++ % 100 == 0) {
                    ui_change_mode(DANGER_MODE);
                    app_tts_play("心率异常, 请及时就医");
                }
            } else {
                heart_cnt = 0;
            }

            if ( max30102_data.spo2 < 90 && max30102_data.spo2 > 30 ) {
                if (o2_cnt++ % 100 == 0) {
                    ui_change_mode(DANGER_MODE);
                    app_tts_play("血氧异常, 请及时就医");
                }
            } else {
                o2_cnt = 0;
            }
        }
    }
}

void motion_init(void)
{
    xTaskCreate(motion_task, "motion_task", 4096, NULL, 10, NULL);
}