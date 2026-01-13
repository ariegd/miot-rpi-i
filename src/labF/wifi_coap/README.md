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
