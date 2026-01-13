/* wifim_comp.c - NODO HIJO (Envía Alerta) */
#include <string.h>
#include <inttypes.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "esp_mesh_internal.h"
#include "mesh_light.h"
#include "nvs_flash.h"
#include "freertos/queue.h"

// Definición de Cola Externa
extern QueueHandle_t mesh_tx_queue;
QueueHandle_t mesh_tx_queue = NULL;

#define RX_SIZE          (1500)
#define TX_SIZE          (1460)

static const char *MESH_TAG = "WIFI_MESH_NODE";
static const uint8_t MESH_ID[6] = { 0x77,0x77,0x77,0x77,0x77, 0xB};
static uint8_t tx_buf[TX_SIZE] = { 0, };
static uint8_t rx_buf[RX_SIZE] = { 0, };
static bool is_running = true;
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static esp_netif_t *netif_sta = NULL;

/*******************************************************
 * Function Definitions
 *******************************************************/

void esp_mesh_p2p_tx_main(void *arg)
{
    esp_err_t err;
    mesh_data_t data;
    mesh_addr_t root_addr = {0}; // Para almacenar la dirección del Root
    char queue_msg[100]; 
    
    data.proto = MESH_PROTO_BIN;
    data.tos = MESH_TOS_P2P;
    is_running = true;

    ESP_LOGI(MESH_TAG, "Tarea TX iniciada. Esperando alertas en la cola...");

    while (is_running) {
        // Bloqueante hasta recibir de la cola
        if (xQueueReceive(mesh_tx_queue, queue_msg, portMAX_DELAY) == pdTRUE) {
            
            // Verificamos conectividad antes de intentar enviar
            if (!is_mesh_connected) {
                ESP_LOGW(MESH_TAG, "Mesh no conectada, descartando mensaje...");
                continue;
            }

            // MODIFICACIÓN CRÍTICA:
            // Si somos ROOT, no tiene sentido enviarnos a nosotros mismos en este esquema UPSTREAM.
            // Si NO somos root, queremos enviar AL root.
            if (esp_mesh_is_root()) {
                 ESP_LOGW(MESH_TAG, "Soy ROOT, no envío alertas upstream.");
                 continue;
            }

            // Obtener la dirección MAC del nodo raíz
            err = esp_mesh_get_root_bssid(&root_addr);
            if (err != ESP_OK) {
                ESP_LOGE(MESH_TAG, "No se pudo obtener la dirección del Root (err=0x%x)", err);
                continue;
            }

            // Preparar payload
            int msg_len = strlen(queue_msg) + 1; // +1 para incluir el null terminator
            if (msg_len > TX_SIZE) msg_len = TX_SIZE;
            memcpy(tx_buf, queue_msg, msg_len);
            
            data.size = msg_len;
            data.data = tx_buf;

            ESP_LOGW(MESH_TAG, "Enviando alerta '%s' al ROOT ("MACSTR")", queue_msg, MAC2STR(root_addr.addr));

            // Enviar UNICAST directamente al Root
            err = esp_mesh_send(&root_addr, &data, MESH_DATA_P2P, NULL, 0);
            
            if (err) {
                ESP_LOGE(MESH_TAG, "Error enviando a Root: 0x%x", err);
            } else {
                ESP_LOGI(MESH_TAG, "Enviado con éxito.");
            }
        }
    }
    vTaskDelete(NULL);
}

void esp_mesh_p2p_rx_main(void *arg)
{
    // RX simple para mantener el stack vivo, aunque este nodo principalmente envía
    esp_err_t err;
    mesh_addr_t from;
    mesh_data_t data;
    int flag = 0;
    data.data = rx_buf;
    data.size = RX_SIZE;
    is_running = true;

    while (is_running) {
        data.size = RX_SIZE;
        err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
        if (err == ESP_OK && data.size > 0) {
            // Procesar si recibimos órdenes del root (opcional)
        }
    }
    vTaskDelete(NULL);
}

esp_err_t esp_mesh_comm_p2p_start(void)
{
    static bool is_comm_p2p_started = false;
    if (!is_comm_p2p_started) {
        is_comm_p2p_started = true;
        xTaskCreate(esp_mesh_p2p_tx_main, "MPTX", 3072, NULL, 5, NULL);
        xTaskCreate(esp_mesh_p2p_rx_main, "MPRX", 3072, NULL, 5, NULL);
    }
    return ESP_OK;
}

void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    mesh_addr_t id = {0,};
    static uint16_t last_layer = 0;

    switch (event_id) {
    case MESH_EVENT_STARTED: {
        esp_mesh_get_id(&id);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:"MACSTR"", MAC2STR(id.addr));
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_STOPPED: {
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
    }
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
        mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
        esp_mesh_get_id(&id);
        mesh_layer = connected->self_layer;
        memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:"MACSTR"%s, ID:"MACSTR"",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr));
        last_layer = mesh_layer;
        is_mesh_connected = true;
        if (esp_mesh_is_root()) {
            esp_netif_dhcpc_stop(netif_sta);
            esp_netif_dhcpc_start(netif_sta);
        }
        esp_mesh_comm_p2p_start();
    }
    break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_PARENT_DISCONNECTED>");
    }
    break;
    default:
        break;
    }
}

void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(MESH_TAG, "<IP_EVENT_STA_GOT_IP>IP:" IPSTR, IP2STR(&event->ip_info.ip));
}

void wifim_task(void *pvParameters)
{
    ESP_ERROR_CHECK(mesh_light_init());
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL));
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_mesh_init());
    ESP_ERROR_CHECK(esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mesh_set_topology(CONFIG_MESH_TOPOLOGY));
    ESP_ERROR_CHECK(esp_mesh_set_max_layer(CONFIG_MESH_MAX_LAYER));
    ESP_ERROR_CHECK(esp_mesh_set_vote_percentage(1));
    ESP_ERROR_CHECK(esp_mesh_set_xon_qsize(128));
    ESP_ERROR_CHECK(esp_mesh_disable_ps());
    ESP_ERROR_CHECK(esp_mesh_set_ap_assoc_expire(10));
    
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    memcpy((uint8_t *) &cfg.mesh_id, MESH_ID, 6);
    cfg.channel = CONFIG_MESH_CHANNEL;
    cfg.router.ssid_len = strlen(CONFIG_MESH_ROUTER_SSID);
    memcpy((uint8_t *) &cfg.router.ssid, CONFIG_MESH_ROUTER_SSID, cfg.router.ssid_len);
    memcpy((uint8_t *) &cfg.router.password, CONFIG_MESH_ROUTER_PASSWD, strlen(CONFIG_MESH_ROUTER_PASSWD));
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE));
    cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
    cfg.mesh_ap.nonmesh_max_connection = CONFIG_MESH_NON_MESH_AP_CONNECTIONS;
    memcpy((uint8_t *) &cfg.mesh_ap.password, CONFIG_MESH_AP_PASSWD, strlen(CONFIG_MESH_AP_PASSWD));
    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));
    
    // NODO NORMAL (NO ROOT FIJO)
    ESP_ERROR_CHECK(esp_mesh_fix_root(false));
    
    // Crear Cola
    mesh_tx_queue = xQueueCreate(5, 100 * sizeof(char)); 
    
    ESP_ERROR_CHECK(esp_mesh_start());
    ESP_LOGI(MESH_TAG, "mesh starts successfully");
    
    vTaskDelete(NULL);
}

void wifim_start(void)
{
    xTaskCreate(&wifim_task, "wifim_task", 4096, NULL, 2, NULL);
}
