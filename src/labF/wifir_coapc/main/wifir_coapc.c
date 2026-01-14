#include <stdio.h>
#include "esp_log.h"
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

// ---  TRES LÍNEAS ---
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
// -------------------------------

#include "coapc_comp.h"
#include "wifir_comp.h"

static const char *TAG = "wifi_coap";

void app_main(void)
{
  /*
    ESP_LOGI(TAG, "Iniciando sistema wifi_coap...");
    wifir_start();
    coapc_start();
    */
    
    // 1. Iniciar recursos del sistema (NVS y Event Loop) UNA SOLA VEZ aquí
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Iniciando sistema wifi_coap...");

    // 2. Iniciar el Mesh / Wi-Fi (Quien da la conectividad)
    wifir_start(); 

    // 3. Iniciar CoAP
    // IMPORTANTE: El cliente CoAP fallará si intenta enviar datos antes de tener IP.
    // Lo ideal es esperar un evento de IP_EVENT_STA_GOT_IP o MESH_EVENT_ROOT_GOT_IP.
    // Como solución rápida, un delay o deja que la tarea CoAP reintente internamente.
    
    vTaskDelay(pdMS_TO_TICKS(5000)); // Esperar un poco a que arranque el Mesh (opcional)
    coapc_start();
}
