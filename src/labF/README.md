| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

```
Máster IoT, curso 25-26
 	└── Autor
 		    └── Ariel Gámez <arielg01@ucm.es>
```
[repositorio](https://github.com/ariegd/miot-aniot/tree/labF/src/labF) en GitHub

# LABF. Práctica ﬁnal de integración
```
El sistema debe estar compuesto por al menos:

* 1 sensor IoT basado en un ESP32 que implemente el detector de proximidad y el
servidor GATT.
* 1 nodo Wi-Fi Mesh regular que implemente el cliente GATT y se conecte a la red Mesh.
Una vez conectado, deberá encapsular y enviar la alerta a través de la red Mesh hacia
el nodo Mesh raíz.
* 1 nodo Wi-Fi Mesh raíz encargado de recibir las alertas y encaminarlas al gateway/hub
IoT mediante Wi-Fi y CoAP.
* 1 PC actuando como gateway/hub IoT que ejecute un servidor CoAP simple.
```

## Objetivo del labF
```
detector de proximidad           -->            nodo WiFi Mesh regular          -->         nodo WiFi Mesh raíz      -->        gateway/hub (PC servidor CoAP)
y servidor GATT                   (GATT)                   y cliente GATT            (Wi-Fi Mesh)                                       (Wi-Fi y CoAP)
```

## Directorio del proyecto
A continuación se muestra una explicación de los archivos en la carpeta del proyecto.
```
├── gattc_wifih
├── gatts_tourch
├── server
├── wifir_coapc
└── README.md                  
```

## Cómo ejecutar el proyecto
Orden de ejecución en cada placa de prototipo, se necesita de 3 a 4 ESP32:
1. [gatts_tourch](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/gatts_touch)
2. [gattc_wifih](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/gattc_wifih)
3. [wifir_coapc](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
4. [server](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/server)

Antes de configurar y construir el proyecto, asegúrese de configurar el chip objetivo correcto utilizando `idf.py set-target <chip_name>`.

### Hardware requerido
1. [gatts_tourch](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/gatts_touch)
* Una placa de desarrollo con ESP32/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.).
* Un cable USB para alimentación y programación.

2. [gattc_wifih](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/gattc_wifih)
* Una placa de desarrollo con ESP32/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.).
* Un cable USB para alimentación y programación.

3. [wifir_coapc](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/wifir_coapc)
* Una placa de desarrollo con ESP32/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.).
* Un cable USB para alimentación y programación.

4. [server](https://github.com/ariegd/miot-rpi-i/tree/labF/src/labF/server)
Servidor CoAP en ESP43:
* Una placa de desarrollo con ESP32/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.).
* Un cable USB para alimentación y programación.

O Servidor CoAP en  Ordenador:
* Portátil
