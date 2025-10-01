#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_chip_info.h"   // <-- chip info está aquí ahora
#include "esp_flash.h"
#include "esp_log.h"

#include "esp_wifi.h"       // <--info de WiFi

void print_wifi_info() {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        printf("=== Información de la conexión WiFi ===\n");
        printf("SSID: %s\n", ap_info.ssid);
        printf("RSSI: %d dBm\n", ap_info.rssi);
    } else {
        printf("⚠️  No hay conexión WiFi activa.\n");
    }
}

void app_main(void)
{
    printf("Hello world!\n");
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    /* Print chip WiFi information */
    printf("Chip %s con %d núcleos\n", CONFIG_IDF_TARGET, chip_info.cores);

    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
        printf("✅ Este SoC tiene WiFi.\n");
        print_wifi_info();
    } else {
        printf("ℹ️ Este SoC no tiene capacidades WiFi.\n");
    }

    /* Print chip information */
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    // Flash size
    uint32_t size_flash_chip;
    if (esp_flash_get_size(NULL, &size_flash_chip) == ESP_OK) {
        printf("Flash size: %luMB\n", size_flash_chip / (1024 * 1024));
    } else {
        printf("Failed to get flash size\n");
    }

    printf("Minimum free heap size: %lu bytes\n",
           (unsigned long) esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(10000/ portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
