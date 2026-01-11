/*
 * 1 sensor IoT basado en un ESP32 que implemente el detector de proximidad y el
 * servidor GATT.
 *
 */

#include <stdio.h>
#include "esp_log.h"
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "gatt_server_comp.h"
#include "touch_pad_comp.h"

static const char *TAG = "gatt_touch_server";

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema gatt_touch_server ...");
    gatt_server_start();
    touch_pad_start();

}
