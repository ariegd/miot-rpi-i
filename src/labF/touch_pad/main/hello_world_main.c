/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

// Ejemplo de Touch Pad
#include "driver/touch_pad.h"
#include "esp_log.h"

#define TOUCH_PAD    TOUCH_PAD_NUM2   // canal que corresponde a un GPIO táctil (por ej. T2 = GPIO2 en ESP32)
#define TOUCH_THRESH_FACTOR  0.8f     // 80 % del valor en reposo

static const char *TAG = "touch_example";

void app_main(void)
{
    uint16_t touch_value;
    uint16_t touch_baseline;
    uint16_t touch_threshold;

    // 1. Inicializar touch pad
    touch_pad_init();

    // 2. Configurar el canal táctil (umbral HW a 0 si se usa detección por SW)
    touch_pad_config(TOUCH_PAD, 0);

    // (Opcional) esperar un poco a que se estabilice
    vTaskDelay(pdMS_TO_TICKS(100));

    // 3. Calibrar: leer valor en reposo (sin tocar)
    touch_pad_read(TOUCH_PAD, &touch_baseline);
    ESP_LOGI(TAG, "Baseline: %u", touch_baseline);

    // 4. Fijar umbral de software
    touch_threshold = (uint16_t)(touch_baseline * TOUCH_THRESH_FACTOR);

    while (1) {
        touch_pad_read(TOUCH_PAD, &touch_value);

        // En ESP32: valor menor que el umbral => tocado/proximidad
        bool touched = (touch_value < touch_threshold);

        ESP_LOGI(TAG, "value=%u, touched=%d", touch_value, touched);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
