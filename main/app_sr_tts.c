/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdio.h>
#include <esp_system.h>
#include <inttypes.h>
#include <sdkconfig.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "driver/gpio.h"

#include "esp_tts.h"
#include "esp_tts_voice_xiaole.h"
#include "esp_tts_voice_template.h"
#include "esp_tts_player.h"
#include "esp_partition.h"

#include "app_sr_tts.h"

/* Buffer for reading/writing to I2S driver. Same length as SPIFFS buffer and I2S buffer, for optimal read/write performance.
   Recording audio data path:
   I2S peripheral -> I2S buffer (DMA) -> App buffer (RAM) -> SPIFFS buffer -> External SPI Flash.
   Vice versa for playback. */
#define SAMPLE_RATE     (8000)
#define EXAMPLE_CHANNEL (1)
#define EXAMPLE_BITS    (16)
#define DEFAULT_VOLUME  (100)

/* Globals */
static const char *TAG = "app_sr_handler";

esp_tts_handle_t *tts_handle = NULL;
esp_codec_dev_handle_t spk_codec_dev = NULL;

esp_err_t app_tts_init(void) {
    /* Initial voice set from separate voice data partition */
    const esp_partition_t* part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "voice_data");

    if (part==NULL) {
        ESP_LOGE(TAG, "Couldn't find voice data partition!");
        return 0;
    } else {
        ESP_LOGI(TAG, "voice_data paration size:%ld", part->size);
    }

    const void* voicedata;

    esp_partition_mmap_handle_t mmap;
    esp_err_t err = esp_partition_mmap(part, 0, part->size, ESP_PARTITION_MMAP_DATA, &voicedata, &mmap);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Couldn't map voice data partition!");
        return ESP_FAIL;
    }

    esp_tts_voice_t *voice = esp_tts_voice_set_init(&esp_tts_voice_template, (int16_t*)voicedata);

    tts_handle = esp_tts_create(voice);
    if (tts_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create TTS handle!");
        return ESP_FAIL;
    }

    spk_codec_dev = bsp_audio_codec_speaker_init();
    assert(spk_codec_dev);

    esp_codec_dev_sample_info_t fs = {
        .sample_rate        = SAMPLE_RATE,
        .channel            = EXAMPLE_CHANNEL,
        .bits_per_sample    = EXAMPLE_BITS,
    };
    esp_codec_dev_close(spk_codec_dev);
    esp_codec_dev_open(spk_codec_dev, &fs);

    gpio_pullup_en(BSP_I2S_DOUT);
    gpio_set_drive_capability(BSP_I2S_DOUT, GPIO_DRIVE_CAP_3);

    return ESP_OK;
}

esp_err_t app_tts_play(const char *prompt1)
{
    bsp_audio_enable(true);

    esp_codec_dev_set_out_vol(spk_codec_dev, DEFAULT_VOLUME);

    /*Play prompt text */
    if (esp_tts_parse_chinese(tts_handle, prompt1)) {
            int len[1]={0};
            do {
                short *pcm_data=esp_tts_stream_play(tts_handle, len, 1);
                for(int i = 0; i < len[0]; i++) {
                    pcm_data[i] = pcm_data[i] * 3;
                }
                int error_code = esp_codec_dev_write(spk_codec_dev, pcm_data, len[0]*2);
            } while(len[0]>0);
    }

    esp_tts_stream_reset(tts_handle);

    return ESP_OK;
}