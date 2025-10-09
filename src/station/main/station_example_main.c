/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   * -----------------------------------
   * Ejemplo de WiFi softAP & station 
   * ------------------------------------
   */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"  // <-- Agregado 
#include "esp_wifi_default.h"
#include "esp_mac.h" // <-- Agregado 
#include "lwip/err.h"
#include "lwip/sys.h"

/* STA Config */
#define EXAMPLE_ESP_WIFI_STA_SSID              CONFIG_ESP_WIFI_REMOTE_AP_SSID
#define EXAMPLE_ESP_WIFI_STA_PASSWD      CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY         CONFIG_ESP_MAXIMUM_STA_RETRY

/* AP Config */
#define EXAMPLE_ESP_WIFI_AP_SSID       CONFIG_ESP_WIFI_AP_SSID
#define EXAMPLE_ESP_WIFI_AP_PASSWD     CONFIG_ESP_WIFI_AP_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL       CONFIG_ESP_WIFI_AP_CHANNEL
#define EXAMPLE_MAX_STA_CONN           CONFIG_ESP_MAX_STA_CONN_AP
/*
#if CONFIG_ESP_STATION_EXAMPLE_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_STATION_EXAMPLE_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_STATION_EXAMPLE_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
*/
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG_AP = "WiFi SoftAP"; // <-- Agregado para el menucofig y salida por consola
static const char *TAG_STA = "WiFi Sta";

static int s_retry_num = 0;


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_STA, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_STA,"connect to the AP fail");
        
        /*
        * Solamente muestra por consola que se ha invocado los EVENT
        * Si se desea agregar algo más de lógica para tratar los EVENT, este es el lugar.
        */
    }   else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)  {
        ESP_LOGI(TAG_STA, "[wifi_event_handler] STA ahora conectado al AP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP)  {
        ESP_LOGI(TAG_STA, "[wifi_event_handler] STA perdida de IP");
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
   if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_STA, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
       /*
        * Ahora se lanza un EVENT cuando se pierde la IP
        * También aquí es donde se agrega la lógica para tratar el caso
        */
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)  {
      ESP_LOGW(TAG_STA, "[ip_event_handler] Hemos perdido la dirección IP");
    }
}

// Creamos otra función para manejar los eventos AP
static void ap_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(TAG_AP, "Station "MACSTR" left, AID=%d, reason:%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}


//2) --- Cambiamos wifi_init_sta() a Modo mixto: AP + STA por ejemplo wifi_init_apsta()  ---
void wifi_init_apsta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    //3) Iniciamos las dos (STA y AP)
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
/* Por que se elimina?
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
  */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));  // &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,          // Se ha cambiado de  IP_EVENT_STA_GOT_IP por ESP_EVENT_ANY_ID
                                                        &ip_event_handler,
                                                        NULL,
                                                        NULL)); //  &instance_got_ip));
   ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,         
                                                        &ap_event_handler,
                                                        NULL,
                                                        NULL));
  
    //4)  Configurando las estructuras de las interfaces   
    // --- Configuración AP 
        wifi_config_t wifi_ap_config = {
          .ap = {
              .ssid = EXAMPLE_ESP_WIFI_AP_SSID,
              .ssid_len = strlen(EXAMPLE_ESP_WIFI_AP_SSID),
              .channel = EXAMPLE_ESP_WIFI_CHANNEL,
              .password = EXAMPLE_ESP_WIFI_AP_PASSWD,
              .max_connection = EXAMPLE_MAX_STA_CONN,
              .authmode = WIFI_AUTH_WPA2_PSK,
              .pmf_cfg = {
                  .required = false,
              },
          },
    };
    if (strlen(EXAMPLE_ESP_WIFI_AP_PASSWD) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    //5) --- Modo mixto  importante ---
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config) );
    
    // --- Configuración STA ---
    wifi_config_t wifi_sta_config = {
        .sta = {
        /* por qué?
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
             .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
            */
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .ssid = EXAMPLE_ESP_WIFI_STA_SSID,
            .password = EXAMPLE_ESP_WIFI_STA_PASSWD,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };
     //6) --- Configuración para cada interfaz
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );


    ESP_LOGI(TAG_AP, "[wifi_init_apsta] finalizado, entonces STA conecta SSID:%s, AP conecta SSID:%s", 
                            EXAMPLE_ESP_WIFI_AP_SSID, EXAMPLE_ESP_WIFI_STA_SSID);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s",
                EXAMPLE_ESP_WIFI_STA_SSID, EXAMPLE_ESP_WIFI_STA_PASSWD);    // <-- Cambia  EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, " Failed to connect to SSID:%s, password:%s",
                  EXAMPLE_ESP_WIFI_STA_SSID, EXAMPLE_ESP_WIFI_STA_PASSWD);
    } else {
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
  /*
    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    }
*/
    ESP_LOGI(TAG_AP, "[app_main] ESP_WIFI_MODE_APSTA (modo mixto)");
    wifi_init_apsta();
}
