#include <stdio.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "bsp/esp-bsp.h"
#include "vibration.h"
#include "display.h"
#include "max_30102.h"
#include "ui.h"
#include "app_sr_tts.h"
#include "motion.h"

void app_main(void)
{
    vibration_init();
    bsp_i2c_init();
    bsp_display_cfg_t disp_cfg = {
        .lvgl_port_cfg = {
            .task_priority = 5,
            .task_stack = 10 * 1024,
            .task_affinity = -1,
            .task_max_sleep_ms = 500,
            .timer_period_ms = 2,
        },
    };
    lv_disp_t* disp = bsp_display_start_with_config(&disp_cfg);
    bsp_display_backlight_on();
    display_init();

    max30102_init();
    ui_init();
    app_tts_init();
    motion_init();
}
