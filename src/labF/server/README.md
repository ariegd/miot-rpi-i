| Supported Targets | Linux |
| ----------------- | ----- | 

# Objetivos del nodo `server`
```
5. Implementación del gateway/hub IoT
* Este dispositivo únicamente debe ejecutar un servidor CoAP que reciba e imprima por
pantalla las alertas de proximidad enviadas por el nodo Mesh raíz.
* Podéis usar cualquier herramienta o librería CoAP para desplegar el servidor en
vuestro PC (p.e. coap-server o la librería aiocoap en Python).
```

##  Tareas en ejecución
* Inicia una tarea FreeRTOS para `coap_example_server`.
   - Crea y contiene la lógica para el cliente CoAP.
o
* Inicia un servidor en python `server_coap.py`.

## Directorio del proyecto
A continuación se muestra una explicación de los archivos en la carpeta del proyecto `wifir_coapc`.
```
├── coap_server                              <-- Servidor CoAP para ESP32
│   ├── CMakeLists.txt
│   ├── dependencies.lock
│   ├── main
│   │   ├── certs
│   │   │   ├── coap_ca.pem
│   │   │   ├── coap_server.crt
│   │   │   └── coap_server.key
│   │   ├── CMakeLists.txt
│   │   ├── coap_server_example_main.c
│   │   ├── idf_component.yml
│   │   ├── Kconfig.projbuild
│   │   └── oscore
│   │       └── coap_oscore.conf
│   ├── partitions.csv
│   ├── README.md
│   ├── sdkconfig
│   ├── sdkconfig.ci
│   ├── sdkconfig.defaults
│   ├── sdkconfig.defaults.esp32h2
│   └── sdkconfig.old
├── README.md
└── server_coap.py                                                  <-- Servidor CoAP para Ordenador

```
## Cómo ejecutar el proyecto
Antes de configurar y construir el proyecto, asegúrese de configurar el chip objetivo correcto utilizando `idf.py set-target <chip_name>`.
Si estamos utilizando el servidor CoAP para el ESP32

### Hardware requerido
Servidor CoAP en ESP32:
* Una placa de desarrollo con ESP32/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.).
* Un cable USB para alimentación y programación.

o 
Servidor CoAP para ordenador:
* Ordenador con Python


### Configuración del proyecto antes de puesta en marcha
Servidor CoAP para ordenador:
1. Requisitos previos. Necesitas tener Python instalado. Abre tu terminal y ejecuta:
```
pip install aiocoap
```
2. Localizar el ip físico del ordenador
```
3: wlp2s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether 74:df:bf:89:72:3b brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.39/24 brd 192.168.1.255 scope global dynamic noprefixroute wlp2s0
       valid_lft 42380sec preferred_lft 42380sec
    inet6 fe80::2677:f441:d0b1:da0e/64 scope link noprefixroute 
       valid_lft forever preferred_lft forever
```
3. Ejecutar
```
 python server_coap.py
🚀 Servidor de Diagnóstico Iniciado
Esperando cualquier mensaje en /Espressif o /espressif...
```

Servidor CoAP en ESP32:
Abrir el menu de configuración del proyecto (`idf.py menuconfig`).

1. En el menu `Example CoAP Configuration  --->`:
* Establecer la configuración de ejemplo.
```
...
(coap://192.168.1.39/Espressif) Target Uri          # Uri del servidor CoAP ya sea en el ESP32 
(sesame) Preshared Key (PSK) to used in the connection to the CoAP server
(password) PSK Client identity (username)
...
```
2. Desactivar el IPv6
* Junto se desactiva `(Top) → Component config → CoAP Configuration ->[ ] Enable Server functionality within CoAP`

### Construir y flashear
Construya el proyecto y fórmelo en la placa, luego ejecute la herramienta de monitorización para ver la salida en serie:
Ejecute `idf.py -p PORT flash monitor` para compilar, actualizar y monitorear el proyecto.
(Para salir del monitor serial, escriba ``Ctrl-]``.)

## Example Output
Servidor CoAP para ordenador:
```
...
🚀 Servidor de Diagnóstico Iniciado
Esperando cualquier mensaje en /Espressif o /espressif...

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
📥 PETICIÓN CAPTURADA
   Método: POST
   Ruta (Path): ()
   Payload: prox_alert 👾
   Desde: 192.168.1.40:53243
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
📥 PETICIÓN CAPTURADA
   Método: POST
   Ruta (Path): ()
   Payload: prox_alert 👾
   Desde: 192.168.1.40:53243
...
```

## Problemas

### Error `UnallowedMethod()`,
Cuando aiocoap recibe una petición a una ruta que no conoce, a veces responde con este error genérico. El culpable más probable es que tu cliente ESP32 está enviando la ruta con una / inicial (ej: /Espressif), y Python la está buscando sin ella, o viceversa.

Vamos a usar un "Servidor Maestro" que acepta CUALQUIER ruta y CUALQUIER método. Esto nos permitirá ver qué está enviando exactamente el ESP32.
