| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Objetivos de este Nodo
```
4. Implementación del nodo Mesh raíz
* Por su parte, el nodo Mesh raíz se conectará a un punto de acceso Wi-Fi (p.e. el de
vuestra casa). Cabe destacar que en el ejemplo examples/mesh/internal_communication
todos los nodos de la red Mesh intentarán primero conectarse al punto de acceso y, tras 
elegirse el nodo raíz, se conectarán  directamente entre ellos. Por tanto, o bien desplegáis 
primero el nodo raíz, o bien modiﬁcáis su código y el de los nodos Mesh regulares para 
ﬁjar explícitamente cuál será el nodo raíz. 
* Además, el nodo raíz deberá ser capaz de recibir las alertas de proximidad enviadas
por los nodos Mesh regulares mediante la función esp_mesh_recv(), ejecutada dentro
de una tarea FreeRTOS.
* Finalmente, el nodo Mesh raíz reenviará las alertas recibidas al gateway/hub IoT
mediante una pe,ción POST de CoAP. 
```

## Nodo wifi_coap
Starts a FreeRTOS task to print "Hello World".

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## How to use example

Follow detailed instructions provided specifically for this example.

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)


## Example folder contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.

## Problemas detectados

### Tome ese payload y lo coloque en la cola `coap_alert_queue`.
La idea es que cada vez que el nodo raíz (Root) reciba un paquete de un nodo hijo, tome ese payload y lo coloque en la cola `coap_alert_queue`.
¿Qué sucede ahora?
* Evento puntual: El cliente CoAP no enviará nada mientras el Mesh esté en silencio.
* Transmisión: En el instante en que un nodo hijo envíe datos (por ejemplo, una lectura de sensor), el Root los recibe, los mete en la cola, y la tarea CoAP los procesa inmediatamente para enviarlos al servidor mediante un POST.

### No es recomendable poner un bucle de recepción dentro de `wifir_task`
porque esa tarea debe terminar después de configurar el Mesh. El lugar correcto para procesar los mensajes y enviar la alerta es en la tarea que ya tienes creada para eso: esp_mesh_p2p_rx_main.

### Cola de Mensajes (FreeRTOS Queue)
Para lograr que el componente Mesh (wifir_comp.c) envíe datos al componente CoAP (coapc_comp.c) solo cuando ocurre un evento puntual, la arquitectura correcta es utilizar una Cola de Mensajes (FreeRTOS Queue).

Esto evita tener un bucle enviando datos constantemente y permite que la tarea CoAP "duerma" o haga mantenimiento hasta que el Mesh le avise.

### Error `ESP_ERR_INVALID_STATE (0x103)` en la función `esp_event_loop_create_default()`
El error ESP_ERR_INVALID_STATE (0x103) en la función esp_event_loop_create_default() ocurre porque estás intentando crear el bucle de eventos dos veces.

En ESP-IDF, el "Event Loop" es un recurso global. Si ya lo has inicializado en el app_main (como te recomendé en el paso anterior), cualquier intento posterior de un componente por crearlo de nuevo hará que el programa aborte.

### Error se produce porque `example_connect()` ha fallado
1. Causa Inmediata (El Crash): En el log ves: WiFi Connect failed 7 times, stop reconnect. seguido de ESP_ERROR_CHECK failed... abort(). Esto ocurre dentro de coapc_start, donde llamas a example_connect(). Como no logra conectar con el SSID "myssid" (posiblemente porque no existe o la clave está mal en el menuconfig), la función devuelve error y el programa explota.
2. Causa Raíz (Conflicto de Arquitectura): Estás uniendo dos mundos que chocan:
* coapc_comp.c (Cliente CoAP): Usa example_connect(). Esta función está diseñada para demos sencillas; inicializa el Wi-Fi, lo configura como Estación (STA) y bloquea el programa hasta obtener IP.
* wifir_comp.c (Mesh/Repeater): El Wi-Fi Mesh (esp_mesh) necesita control total sobre el driver Wi-Fi para configurar las interfaces AP y STA dinámicamente.

El problema: Al llamar primero a coapc_start(), este intenta configurar el Wi-Fi como una estación normal. Si falla, crashea. Si tuviera éxito, bloquearía o interferiría con la configuración posterior de wifir_start().

### Error fatal error: esp_wifi.h: No such file or directory
El error fatal error: esp_wifi.h: No such file or directory ocurre porque el sistema de compilación de ESP-IDF (CMake) no sabe que tu nuevo componente coapc_comp depende de las librerías de Wi-Fi del sistema.

### Desactivación de IPv6 de forma permanente
¿Por qué hacerlo aquí?
* Persistencia: Al estar en sdkconfig.defaults, cualquier persona que clone tu proyecto o cualquier nueva compilación desde cero tomará este valor por defecto.
* Prioridad: Cuando ejecutas idf.py build, el sistema lee este archivo para generar el sdkconfig final.
* Limpieza: Evitas que el servidor intente levantar interfaces IPv6 que no vas a usar, ahorrando memoria en el heap (algo importante para evitar el desbordamiento que mencionamos antes).


### Error 4.04: Not Found que recibe el cliente y la falta de logs
El error 4.04: Not Found que recibe el cliente y la falta de logs en el servidor confirman lo siguiente:
1. La conexión es correcta: El cliente llega al servidor (IP 192.168.1.42), por eso recibes una respuesta 4.04 en lugar de un error de "Timeout".
2. El problema: Estás recibiendo un Not Found porque el cliente está pidiendo una ruta que no existe.

### El `coap_server` se queda en silencio y el `coap_client` no imprime nada después de conectarse al Wi-Fi.
**El paso clave: Desactivar IPv6 (si no lo usas)**
Muchos ejemplos de ESP-IDF esperan a que la interfaz de red tenga una IP IPv6 "Local Link" antes de disparar el evento que arranca la tarea CoAP.

Si tu router solo da IPv4, la tarea nunca empieza. Para arreglarlo:
1. Ejecuta `idf.py menuconfig` en ambos proyectos.
2. Ve a Example Connection Configuration.
3. Desmarca (u oculta) la opción de IPv6 Support si está habilitada, o asegúrate de que el código no dependa de `EXAMPLE_CONNECT_PREFERRED_IPV6`.

coap_server:
```
I (3371) wifi:[ADDBA]RX addba response, status:0, tid:5/tb:1(0xa1), bufsize:32, batimeout:0, txa_wnd:32
W (3611) wifi:<ba-add>idx:0, ifx:0, tid:0, TAHI:0x100bf3c, TALO:0x918f64a0, (ssn:2, win:64, cur_ssn:2), CONF:0xc0000005
I (4881) esp_netif_handlers: example_netif_sta ip: 192.168.1.42, mask: 255.255.255.0, gw: 192.168.1.1
I (4881) example_connect: Got IPv4 event: Interface "example_netif_sta" address: 192.168.1.42
I (4881) example_common: Connected to example_netif_sta
I (4891) example_common: - IPv4 address: 192.168.1.42,
I (4891) CoAP_server: Iniciando Servidor CoAP...
I (4901) CoAP_server: !!! SERVIDOR COAP ESCUCHANDO EN PUERTO 5683 !!!
I (4911) main_task: Returned from app_main()
```

coap_client:
```
I (3071) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (3081) wifi:set rx beacon pti, rx_bcn_pti: 0, bcn_timeout: 25000, mt_pti: 0, mt_time: 10000
I (3091) wifi:AP's beacon interval = 102400 us, DTIM period = 3
I (3301) wifi:<ba-add>idx:0 (ifx:0, a0:64:8f:91:3c:bf), tid:0, ssn:2, winSize:64
I (4591) esp_netif_handlers: example_netif_sta ip: 192.168.1.40, mask: 255.255.255.0, gw: 192.168.1.1
I (4591) example_connect: Got IPv4 event: Interface "example_netif_sta" address: 192.168.1.40
I (4591) example_common: Connected to example_netif_sta
I (4601) example_common: - IPv4 address: 192.168.1.40,
I (4601) CoAP_client: !!! TAREA CLIENTE INICIADA CON URI: coap://192.168.1.42/Espressif !!!
I (4611) CoAP_client: DNS lookup succeeded. IP=192.168.1.42
I (4621) main_task: Returned from app_main()
Received:
Hello World!
I (4831) CoAP_client: 10... 
I (5831) CoAP_client: 9... 
I (6831) CoAP_client: 8... 
I (7831) CoAP_client: 7... 
I (8831) CoAP_client: 6... 
I (9831) CoAP_client: 5... 
I (10831) CoAP_client: 4... 
I (11831) CoAP_client: 3... 
I (12831) CoAP_client: 2... 
I (13831) CoAP_client: 1... 
I (14831) CoAP_client: 0... 
I (15831) CoAP_client: Starting again!
Received:
Hello World!
I (16191) CoAP_client: 10... 
I (17191) CoAP_client: 9... 
```

### Solamente el NODO RAIZ (ROOT) 
1. Debe estar conectado con la wifi router (el ruter de casa) 
2. Una vez que ya se tiene designado el NODO RAIZ el resto de los nodos mesh se conectan automaticamente.

### **2. Direccionamiento: ¿A quién envía el hijo?**
Un nodo hijo tiene tres formas principales de enviar información:

* **Hijo a Root (Upstream):** Es lo más común (por ejemplo, enviar datos de un sensor al servidor central). Para esto, se usa la dirección del Root o se indica que el destino es el "External IP Network".  
* **Hijo a Hijo (P2P):** Un hijo puede enviar un mensaje directamente a otro nodo si conoce su dirección MAC.  
* **Hijo a Padre:** Puede comunicarse con su nodo superior inmediato.

### **El Envío (Transmisión)**
Se gestiona en la función `esp_mesh_p2p_tx_main`(Línea 65).

* **Quién envía:** Solo el nodo **ROOT** (Raíz) envía estos paquetes masivos. El código tiene un condicional: `if (!esp_mesh_is_root()) { continue; }`.  
* A quién: El Root obtiene la tabla de rutas (`esp_mesh_get_routing_table`) y recorre todos los nodos conectados para enviarles un paquete individual (Unicast) usando la función:  
  `esp_mesh_send(&route_table[i], &data, MESH_DATA_P2P, NULL, 0); ` 
* **Qué envía:** Envía un buffer de 1460 bytes (TX_SIZE) que contiene comandos para encender o apagar un LED (light_on / light_off).

### El error ps:1 (Power Save: Enabled).
El error está clarísimo en el log y es lo que sospechábamos en la respuesta anterior, pero ahora tenemos la confirmación exacta:
```
I (644) mesh_main: mesh starts successfully, heap:163168, root fixed<0>(tree), ps:1
```
El culpable es `ps:1` (Power Save: Enabled).

### ¿Por qué falla?
El log muestra que tu router (Movistar) está usando un ancho de banda ancho de 40MHz (secondary channel offset:1(40U)). Cuando el Nodo Raíz tiene el ahorro de energía activado (ps:1), apaga su radio Wi-Fi intermitentemente para ahorrar batería.

Al intentar conectar con un router moderno en modo Mesh con 40MHz, el router intenta negociar la conexión, pero el ESP32 está "dormido" en los microsegundos clave del handshake de seguridad. Resultado: [FAIL]root:0.
