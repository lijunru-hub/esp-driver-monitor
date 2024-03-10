/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "max30102.h"
#include "driver/i2c.h"
#include "ui.h"

#if CONFIG_BSP_I2C_NUM == 1
#define I2C_NUM                     0
#else
#define I2C_NUM                     1
#endif
#define I2S_SDA_PIN 45
#define I2S_SCL_PIN 47
#define I2C_EXPAND_CLK_SPEED_HZ     800000

QueueHandle_t max30102_queue = NULL;

static esp_err_t i2c_init()
{
    i2c_config_t conf = {
        .mode               = I2C_MODE_MASTER,
        .sda_io_num         = I2S_SDA_PIN,
        .sda_pullup_en      = GPIO_PULLUP_ENABLE,
        .scl_io_num         = I2S_SCL_PIN,
        .scl_pullup_en      = GPIO_PULLUP_ENABLE,
        .master.clk_speed   = I2C_EXPAND_CLK_SPEED_HZ
    };
    i2c_param_config(I2C_NUM, &conf);
    i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);
    return ESP_OK;
}

static void max30102_task(void *arg)
{
    max30102_handle_t max30102_handle = NULL;
    max30102_data_t data = {0};
    i2c_init();
    max30102_create(I2C_NUM, &max30102_handle);
    max30102_config(max30102_handle);

    while (1) {
        max30102_get_data(max30102_handle, &data);
        if (data.hand_detected) {
            printf("IR: %f, Red: %f\n", data.heart_rate, data.spo2);
            lv_chart_set_next_value(ui_ChartHR, ui_ChartHR_series_1, data.heart_rate);
            lv_chart_refresh(ui_ChartHR);
            char heart_rate_str[20];
            char spo2_str[20];
            snprintf(heart_rate_str, sizeof(heart_rate_str), "%.1f", data.heart_rate);
            snprintf(spo2_str, sizeof(spo2_str), "%.1f", data.spo2);
            lv_label_set_text(ui_LabelHeartrate, heart_rate_str);
            lv_label_set_text(ui_LabelSao2, spo2_str);
        }
        xQueueSend(max30102_queue, &data, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

esp_err_t max30102_init(void)
{
    max30102_queue = xQueueCreate(10, sizeof(max30102_data_t));
    xTaskCreate(max30102_task, "max30102_task", 1024 * 4, NULL, 5, NULL);
    return ESP_OK;
}