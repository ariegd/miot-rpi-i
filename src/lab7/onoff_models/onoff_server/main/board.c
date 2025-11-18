/* board.c - Board-specific hooks */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "board.h"
#include "led_strip.h"

#define TAG "BOARD"

#define BLINK_GPIO 2
// Variables globales para el manejo de la tira LED
static led_strip_handle_t led_strip;
static uint8_t r_val = 0;
static uint8_t g_val = 0;
static uint8_t b_val = 0;
// Intensidad del brillo (0-255)
#define LED_INTENSITY 20

struct _led_state led_state[3] = {
    { LED_OFF, LED_OFF, LED_R, "red"   },
    { LED_OFF, LED_OFF, LED_G, "green" },
    { LED_OFF, LED_OFF, LED_B, "blue"  },
};

// Función auxiliar para actualizar el pixel físico
static void update_led_strip(void)
{
    if (led_strip) {
        // Establecer el pixel 0 con los valores actuales de R, G y B
        led_strip_set_pixel(led_strip, 0, r_val, g_val, b_val); //
        led_strip_refresh(led_strip); //
    }
}

void board_led_operation(uint8_t pin, uint8_t onoff)
{
  for (int i = 0; i < 3; i++) {
        // Buscamos qué LED lógico se está intentando controlar
        if (led_state[i].pin != pin) {
            continue;
        }

        // Verificar si el estado ya es el deseado
        if (onoff == led_state[i].previous) {
            ESP_LOGW(TAG, "led %s is already %s",
                     led_state[i].name, (onoff ? "on" : "off"));
            return;
        }

        // Actualizamos el estado previo
        led_state[i].previous = onoff;

        // Lógica para asignar color según el pin lógico definido en board.h
        // Si board.h define LED_R como un pin específico, usamos eso para identificar el color
        if (led_state[i].pin == LED_R) {
            r_val = onoff ? LED_INTENSITY : 0;
        } 
        else if (led_state[i].pin == LED_G) {
            g_val = onoff ? LED_INTENSITY : 0;
        } 
        else if (led_state[i].pin == LED_B) {
            b_val = onoff ? LED_INTENSITY : 0;
        }

        // Enviar los nuevos colores a la tira LED
        update_led_strip();
        return;
    }

    ESP_LOGE(TAG, "LED is not found!");
}

static void board_led_init(void)
{
    // Configuración de la tira LED (Adaptado de configure_led en blink_example_main.c)
    ESP_LOGI(TAG, "Initializing LED Strip at GPIO %d", BLINK_GPIO);

    /* LED strip initialization with the GPIO and pixels number */
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO, //
        .max_leds = 1, // Controlamos 1 solo LED en la placa
    };

    /* Configuración del backend RMT (Recomendado para ESP32) */
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    // Inicializar el dispositivo RMT
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip)); //

    // Limpiar el LED al inicio (apagarlo)
    led_strip_clear(led_strip); //
    
    // Reiniciar estados lógicos internos
    r_val = 0; g_val = 0; b_val = 0;
    for (int i = 0; i < 3; i++) {
        led_state[i].previous = LED_OFF;
    }
}

void board_init(void)
{
    board_led_init();
}
