/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Text-to-Speech (TTS) system
 *
 * This function initializes the TTS system by mapping the voice data partition into memory,
 * initializing the voice set with the mapped voice data, and creating a TTS handle.
 *
 * @return ESP_OK if the initialization was successful, ESP_FAIL otherwise.
 */
esp_err_t app_tts_init(void);

/**
 * @brief Play a prompt using the Text-to-Speech (TTS) system
 *
 * This function initializes the speaker codec device, sets the output volume, opens the codec device with the specified sample info,
 * parses the prompt text into Chinese phonemes, and plays the phonemes using the TTS system.
 * After playing the prompt, it closes the codec device and resets the TTS stream.
 *
 * @param prompt1 The prompt text to be played.
 * @return ESP_OK if the prompt was successfully played, ESP_FAIL otherwise.
 */
esp_err_t app_tts_play(const char *prompt1);

#ifdef __cplusplus
}
#endif