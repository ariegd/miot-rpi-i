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

#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "shtc3.h"

#define TAG "BOARD"

/* A ver si funciona el sensor*/
//shtc3_t tempSensor;
//i2c_master_bus_handle_t bus_handle;
/* Variables del sensor */
static shtc3_t tempSensor;
static i2c_master_bus_handle_t bus_handle;
float temperature_c;
float humidity_rh;

struct _led_state led_state[3] = {
    { LED_OFF, LED_OFF, LED_R, "red"   },
    { LED_OFF, LED_OFF, LED_G, "green" },
    { LED_OFF, LED_OFF, LED_B, "blue"  },
};

void board_led_operation(uint8_t pin, uint8_t onoff)
{
    for (int i = 0; i < 3; i++) {
        if (led_state[i].pin != pin) {
            continue;
        }
        if (onoff == led_state[i].previous) {
            ESP_LOGW(TAG, "led %s is already %s",
                     led_state[i].name, (onoff ? "on" : "off"));
            return;
        }
        gpio_set_level(pin, onoff);
        led_state[i].previous = onoff;
        return;
    }

    ESP_LOGE(TAG, "LED is not found!");
}

static void board_led_init(void)
{
    for (int i = 0; i < 3; i++) {
        gpio_reset_pin(led_state[i].pin);
        gpio_set_direction(led_state[i].pin, GPIO_MODE_OUTPUT);
        gpio_set_level(led_state[i].pin, LED_OFF);
        led_state[i].previous = LED_OFF;
    }
}

void board_i2c_init(void) {
        i2c_master_bus_config_t i2c_bus_config = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .i2c_port = I2C_NUM_0,
            .scl_io_num = 8,
            .sda_io_num = 10,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
        };

        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
       shtc3_init(&tempSensor, bus_handle, 0x70);
}

// --- NUEVA FUNCIÓN: Obtiene una sola lectura y retorna ---
esp_err_t board_get_temp(float *t, float *h)
{
    // Llama al driver y guarda valores en los punteros
    return shtc3_get_temp_and_hum_lpm(&tempSensor, t, h);
}

void board_init(void)
{
    board_led_init();
    ESP_LOGI(TAG,"Inicializando I2C y sensor SHTC3...\n");
    board_i2c_init();
    ESP_LOGI(TAG,"Inicialización completa.\n");
}
