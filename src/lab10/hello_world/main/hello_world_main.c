/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "esp_log.h"
#include "esp_event.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "ttn.h"


static const char *TAG = "LoRaWAN_APP";

const char *appEui = "0101010101010101";
// DevEUI
const char *devEui = "70B3D57ED0074CF4";
// AppKey
const char *appKey = "A8A9FE204BB384D205CB3C5611982E9F";

// Pins and other resources
#define TTN_SPI_HOST      SPI2_HOST
#define TTN_SPI_DMA_CHAN  SPI_DMA_DISABLED
#define TTN_PIN_SPI_SCLK  4
#define TTN_PIN_SPI_MOSI  2
#define TTN_PIN_SPI_MISO  3
#define TTN_PIN_NSS       6
#define TTN_PIN_RXTX      TTN_NOT_CONNECTED
#define TTN_PIN_RST       5
#define TTN_PIN_DIO0      0
#define TTN_PIN_DIO1      1

#define TX_INTERVAL 30
static uint8_t msgData[] = {0xAA, 0xBB, 0xCC, 0xDD};


void sendMessages(void* pvParameter)
{
    while (1) {
        ESP_LOGI(TAG, "Sending message...");
        ttn_response_code_t res = ttn_transmit_message(msgData, sizeof(msgData), 1, false);
        if ( res == TTN_SUCCESSFUL_TRANSMISSION ) { ESP_LOGI(TAG, "Message sent.");}else{ ESP_LOGI(TAG, "Transmission failed.");}

        vTaskDelay(TX_INTERVAL * pdMS_TO_TICKS(1000));
    }
}

void messageReceived(const uint8_t* message, size_t length, ttn_port_t port)
{
    ESP_LOGI(TAG, "Message of %d bytes received on port %d:", length, port);
    for (int i = 0; i < length; i++)
        ESP_LOGI(TAG, " %02x", message[i]);
    //printf("\n");
}

void app_main(void)
{
    esp_err_t err;
    // Initialize the GPIO ISR handler service
    err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    ESP_ERROR_CHECK(err);
    
    // Initialize the NVS (non-volatile storage) for saving and restoring the keys
    err = nvs_flash_init();
    ESP_ERROR_CHECK(err);

    // Initialize SPI bus
    spi_bus_config_t spi_bus_config = {
        .miso_io_num = TTN_PIN_SPI_MISO,
        .mosi_io_num = TTN_PIN_SPI_MOSI,
        .sclk_io_num = TTN_PIN_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    }; 
    err = spi_bus_initialize(TTN_SPI_HOST, &spi_bus_config, TTN_SPI_DMA_CHAN);
    ESP_ERROR_CHECK(err);

    // Initialize TTN
    ttn_init();

    // Configure the SX127x pins
    ttn_configure_pins(TTN_SPI_HOST, TTN_PIN_NSS, TTN_PIN_RXTX, TTN_PIN_RST, TTN_PIN_DIO0, TTN_PIN_DIO1);

    // The below line can be commented after the first run as the data is saved in NVS
    ttn_provision(devEui, appEui, appKey);

    // Register callback for received messages
    ttn_on_message(messageReceived);

    // ttn_set_adr_enabled(false);
    // ttn_set_data_rate(TTN_DR_US915_SF7);
    // ttn_set_max_tx_pow(14);

    ESP_LOGI(TAG, "Joining...");
    if (ttn_join())
    {
        ESP_LOGI(TAG, "Joined.");
        xTaskCreate(sendMessages, "send_messages", 1024 * 4, (void* )0, 3, NULL);
    }
    else
    {
        ESP_LOGI(TAG, "Join failed. Goodbye");
    }
}
