#include <stdio.h>
#include "esp_log.h"
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
//#include "gattc_comp.h"
#include "wifir_comp.h"

static const char *TAG = "wifi_coap";

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando sistema wifi_coap...");
    //gattc_start();
    wifir_start();
}
