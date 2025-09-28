# LAB1. Introducción al entorno de desarrollo ESP-IDF

**Objetivos**

* Conocer el entorno de desarrollo para el ESP32.
* Ser capaz de compilar, flashear y monitorizar proyectos sencillos basados en ESP-IDF.
* Entender el funcionamiento básico de una aplicación ESP-IDF que haga uso de las capacidades WiFi del ESP32.
* Personalizar variables de configuración de proyectos ESP-IDF.
* Responder a eventos básicos de red en ESP-IDF.

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
