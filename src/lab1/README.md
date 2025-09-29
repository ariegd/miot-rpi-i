# LAB1. Introducción al entorno de desarrollo ESP-IDF

**Objetivos**

* Conocer el entorno de desarrollo para el ESP32.
* Ser capaz de compilar, flashear y monitorizar proyectos sencillos basados en ESP-IDF.
* Entender el funcionamiento básico de una aplicación ESP-IDF que haga uso de las capacidades WiFi del ESP32.
* Personalizar variables de configuración de proyectos ESP-IDF.
* Responder a eventos básicos de red en ESP-IDF.

## 3 ✅ Cambios claves:
*migrada a ESP-IDF v6.0-dev-2594-g25c40d4563*
* chip_info.cores y chip_info.revision → %d (porque son int).
* flash_size y esp_get_minimum_free_heap_size() → PRIu32 (porque son uint32_t).

## 2 ✅ Resumen de cambios:
*migrada a ESP-IDF v6.0-dev-2594-g25c40d4563*
* #include "esp_chip_info.h" → obligatorio ahora.
* #include "esp_spi_flash.h" eliminado → usar #include "esp_flash.h".
* %d, %lu reemplazados por macros de <inttypes.h> → PRIu32, PRId32, etc.

## 1 🔑 Cambios importantes respecto a tu versión original:
*migrada a ESP-IDF v6.0-dev-2594-g25c40d4563*
* #include "esp_spi_flash.h" ➝ #include "esp_flash.h".
* spi_flash_get_chip_size() ➝ esp_flash_get_size(NULL, &flash_size).
* Uso de uint32_t para el tamaño de la flash.
* printf adaptado a %lu porque flash_size es uint32_t.

## Crear un proyecto básico
```
idf.py create-project blink_led
cd blink_led
```

## Salir del monitor pero que el ESP32 siga corriendo
## reiniciar el ESP32 desde el monitor
```
Ctrl + ]

Ctrl + T  luego  Ctrl + R
```

## Crear y compilar un proyecto de ejemplo 
```
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py -p /dev/ttyUSB0 monitor
```

## Activar y comprobar ESP-IDF (framework oficial de Espressif)
```
. $HOME/esp/esp-idf/export.sh
idf.py --version
```
