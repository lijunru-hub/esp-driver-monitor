#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vibration.h"

#define TAG "vibration"
QueueHandle_t vibration_queue = NULL;
static void vibration_task(void *pvParameters)
{
    vibration_message_t receivedMessage;
    int intensity = 0;
    while (1)
    {
        if (xQueueReceive(vibration_queue, &receivedMessage, portMAX_DELAY) == pdTRUE) {
            intensity = receivedMessage.intensity;
            if (intensity==0) continue;
            // printf ("main %d %d \n",receivedMessage.type, receivedMessage.intensity);
#if Z_axis
            switch (receivedMessage.type) {
                case TOUCH:
                    vibration_control((int)(intensity*0.6), (int)(intensity*0.6), 1, 0, (int)(intensity*0.2), 2);
                    break;
                case PRESS:
                    vibration_control((int)(intensity*0.6), (int)(intensity*0.6), 1, 0, (int)(intensity*0.2), 5);
                    break;
                case LONG_PRESS:
                    vibration_control((int)(intensity*0.8), (int)(intensity*0.4), 1, 0, (int)(intensity*0.15), 5);
                    break;
                case SCROLL:
                    vibration_control((int)(intensity*0.6), (int)(intensity*0.6), 1, 0, (int)(intensity*0.2), 1);
                    break;
                case LONG_VIBRATION:
                    vibration_control((int)intensity, (int)intensity, 5, 0, (int)(intensity/4), 5);
                    break;
            }
#else
            switch (receivedMessage.type) {
                case TOUCH:
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)(intensity), 0);
                    vTaskDelay(pdMS_TO_TICKS(30));
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0, 0);

                    // vibration_control((int)(intensity * 0.8), (int)(intensity * 0.4), 1, 0, (int)(intensity * 0.2), 3);
                    break;
                case PRESS:
                    // vibration_control((int)(intensity*0.8), (int)(intensity*0.4), 1, 0, (int)(intensity*0.2), 3);
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)(intensity), 0);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0, 0);
                    break;
                case LONG_PRESS:
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)(intensity), 0);
                    vTaskDelay(pdMS_TO_TICKS(70));
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0, 0);
                    // vibration_control((int)(intensity*0.8), (int)(intensity*0.4), 20, 0, (int)(intensity*0.15), 5);
                    break;
                case SCROLL:
                // ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)(intensity), 0);
                //     vTaskDelay(pdMS_TO_TICKS(20));
                //     ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0, 0);
                    vibration_control((int)(intensity), (int)(intensity), 1, (int)(intensity-2), 1, 2);
                    vibration_control((int)(intensity), 1, 2, 0, (int)(intensity*0.5), 5);
                    break;
                case LONG_VIBRATION:
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, (int)(intensity), 0);
                    vTaskDelay(pdMS_TO_TICKS(80));
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0, 0);

                    vibration_control((int)intensity, (int)intensity, 50, 0, (int)(intensity/4), 50);
                    break;
            }
#endif

        }
    }
    vTaskDelete(NULL);
}

void vibration_init(void)
{
    vibration_queue = xQueueCreate(5, sizeof(vibration_message_t));
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWM_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    TaskHandle_t pwm_task_handle;
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ_HZ
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .gpio_num = PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = PWM_CHANNEL,
        // .intr_type = LEDC_INTR_FADE_END,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channel_conf);
    ledc_fade_func_install(0);
    ledc_stop(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0);
    gpio_set_level(PWM_PIN, 0);
    xTaskCreate(&vibration_task, "vibration_task", 4 * 1024, NULL, 5, &pwm_task_handle);
}



void vibration_control(uint32_t target_duty1, uint32_t scale1, uint32_t cycle_num1,uint32_t target_duty2, uint32_t scale2, uint32_t cycle_num2)
{
    // printf("vibration %ld %ld %ld %ld\n", target_duty1, scale1, target_duty2, scale2);
    ledc_set_fade_step_and_start(
        LEDC_LOW_SPEED_MODE,
        PWM_CHANNEL,
        target_duty1,     // 设置目标占空比
        scale1,           // 设置步进为负值，即递减
        cycle_num1,       // 定义淡出步进延迟时间
        LEDC_FADE_NO_WAIT // 等待淡出完成
    );
    ledc_set_fade_step_and_start(
        LEDC_LOW_SPEED_MODE,
        PWM_CHANNEL,
        target_duty2,  // 设置目标占空比
        scale2,  // 设置步进为负值，即递减
        cycle_num2, // 定义淡出步进延迟时间
        LEDC_FADE_NO_WAIT  // 等待淡出完成
    );
    // ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,PWM_CHANNEL,300,0);
    // ledc_set_fade_with_step(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0, 300, 1);
    // ledc_fade_start(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, LEDC_FADE_NO_WAIT);
}