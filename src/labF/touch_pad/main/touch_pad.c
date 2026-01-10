#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

// Configuración obtenida mediante Kconfig
#define TOUCH_PAD           ((touch_pad_t)CONFIG_TOUCH_PAD_NUMBER)
// Convertimos el porcentaje entero del Kconfig a factor flotante (80 -> 0.8f)
#define TOUCH_THRESH_FACTOR ((float)CONFIG_TOUCH_THRESH_FACTOR / 100.0f)
#define TIME_THRESHOLD_MS   CONFIG_TIME_THRESHOLD_MS

static const char *TAG = "touch_example";

void app_main(void)
{
    uint16_t touch_value;
    uint16_t touch_baseline;
    uint16_t touch_threshold;

    // Variables para el control de tiempo
    bool is_touching = false;        // Estado actual del toque
    bool alert_sent = false;         // Bandera para no repetir la alerta
    TickType_t start_tick = 0;       // Momento en que empezó el toque

    // Conversión de ms a Ticks de FreeRTOS
    const TickType_t required_ticks = pdMS_TO_TICKS(TIME_THRESHOLD_MS);

    // 1. Inicializar touch pad
    touch_pad_init();

    // 2. Configurar el canal táctil
    touch_pad_config(TOUCH_PAD, 0);

    // Esperar a que se estabilice el hardware
    vTaskDelay(pdMS_TO_TICKS(100));

    // 3. Calibrar: leer valor en reposo
    touch_pad_read(TOUCH_PAD, &touch_baseline);
    ESP_LOGI(TAG, "Calibracion completa. Baseline: %u", touch_baseline);

    // 4. Fijar umbral de software
    touch_threshold = (uint16_t)(touch_baseline * TOUCH_THRESH_FACTOR);
    ESP_LOGI(TAG, "Umbral de disparo: %u", touch_threshold);

    while (1) {
        touch_pad_read(TOUCH_PAD, &touch_value);

        // En ESP32: valor menor que el umbral => tocado
        bool current_touched = (touch_value < touch_threshold);

        if (current_touched) {
            // Si acabamos de empezar a tocar
            if (!is_touching) {
                is_touching = true;
                start_tick = xTaskGetTickCount(); // Guardamos el tiempo actual
                alert_sent = false;               // Reseteamos la alerta
                ESP_LOGD(TAG, "Toque iniciado...");
            } 
            // Si ya estábamos tocando, verificamos el tiempo transcurrido
            else {
                TickType_t current_tick = xTaskGetTickCount();
                
                // Si ha pasado el tiempo requerido Y no hemos enviado la alerta aún
                if (!alert_sent && (current_tick - start_tick >= required_ticks)) {
                    ESP_LOGW(TAG, "ALERTA: Proximidad detectada por 1.5 segundos!");
                    alert_sent = true; // Marcamos para no spamear el log
                    
                    // Aquí podrías encender un LED o activar un buzzer
                }
            }
        } else {
            // Si se deja de tocar, reseteamos todo
            if (is_touching) {
                is_touching = false;
                alert_sent = false;
                ESP_LOGD(TAG, "Toque liberado (antes o después de la alerta).");
            }
        }

        // Log de depuración opcional (comentado para no saturar si no es necesario)
        // ESP_LOGI(TAG, "Val: %u, Tocado: %d", touch_value, current_touched);

        vTaskDelay(pdMS_TO_TICKS(50)); // Muestreo cada 50ms
    }
}
