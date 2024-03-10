#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif


#define Z_axis              (0)
#define PWM_PIN             GPIO_NUM_8
#define PWM_CHANNEL         LEDC_CHANNEL_0
#define PWM_FREQ_HZ         2000
#define PWM_RESOLUTION      LEDC_TIMER_10_BIT

typedef enum {
    TOUCH = 0,
    PRESS = 1,
    LONG_PRESS = 2,
    SCROLL = 3,
    LONG_VIBRATION = 4
} vibration_type_t;

#if Z_axis
    typedef enum {
        STRONG = 512,
        MEDIUM = 300,
        WEAK = 100,
        OFF =0
    } vibration_intensity_t;
#else
    typedef enum {
        OFF =0,
        WEAK = 500,
        MEDIUM = 800,
        STRONG = 1023
    } vibration_intensity_t;
#endif

typedef struct {
    vibration_type_t type;
    vibration_intensity_t intensity;
} vibration_message_t;

extern QueueHandle_t vibration_queue;
void vibration_control(uint32_t target_duty1, uint32_t scale1, uint32_t cycle_num1, uint32_t target_duty2, uint32_t scale2, uint32_t cycle_num2);
void vibration_init(void);
#ifdef __cplusplus
} /*extern "C"*/
#endif

