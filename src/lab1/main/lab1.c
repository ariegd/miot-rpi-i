#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_chip_info.h"   // <-- chip info está aquí ahora
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"       // <--info de WiFi

// #define CONFIG_HELLO_TASK_DELAY 1000
// #define CONFIG_WIFI_TASK_DELAY 2000

static const char *TAG_HELLO = "HELLO_TASK";
static const char *TAG_WIFI  = "WIFI_TASK";


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

void hello_task(void *pvParameter)
{
      esp_chip_info_t chip_info;
      esp_chip_info(&chip_info);

      while (1) {
        printf(" [HELLO] Hello world!\n");   // se puede reemplazar por ESP_LOGI(TAG_HELLO, " Hello world!")
        
        /* Print chip information */

        printf("[HELLO] This is %s chip with %d CPU cores, WiFi%s%s, ",
                CONFIG_IDF_TARGET,
                chip_info.cores,
                (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

        printf("[HELLO] silicon revision %d, ", chip_info.revision);

        // Flash size
        uint32_t size_flash_chip;
        if (esp_flash_get_size(NULL, &size_flash_chip) == ESP_OK) {
            printf("[HELLO] Flash size: %luMB\n", size_flash_chip / (1024 * 1024));
        } else {
            printf("[HELLO] Failed to get flash size\n");
        }

        printf("[HELLO] Minimum free heap size: %lu bytes\n",
               (unsigned long) esp_get_minimum_free_heap_size());  
           
        vTaskDelay(CONFIG_HELLO_TASK_DELAY / portTICK_PERIOD_MS);
    }
}

void wifi_info_task(void *pvParameter)
{
    while (1) {
        printf("[WIFI] WiFi information!\n");
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        /* Print chip WiFi information */
        printf("[WIFI] Chip %s con %d núcleos\n", CONFIG_IDF_TARGET, chip_info.cores);

        if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
            printf("[WIFI] ✅ Este SoC tiene WiFi.\n");
            print_wifi_info();
        } else {
            printf("[WIFI] ℹ️ Este SoC no tiene capacidades WiFi.\n");
        }
        
        vTaskDelay(CONFIG_WIFI_TASK_DELAY / portTICK_PERIOD_MS);
    }
    
}

void app_main(void)
{
    xTaskCreate( &hello_task, "hello_task", 2048, NULL, 5, NULL );
    xTaskCreate( &wifi_info_task, "wifi_info_task", 2048, NULL, 4, NULL );
}
