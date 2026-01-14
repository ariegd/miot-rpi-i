| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Objetivos del nodo `gattc_wifih`
```
3. Implementación del nodo Mesh regular

* Los nodos Mesh que no sean el nodo raíz deben implementar un cliente GATT que
escaneará paquetes de anuncio, se conectará al servidor GATT de uno de los sensores
IoT (del único que tendremos), descubrirá el servicio y la característica expuestos por
este y se suscribirá para recibir notiﬁcaciones a través del descriptor CCC.
* Al mismo tiempo, estos nodos formarán una red Wi-Fi Mesh que ampliará la cobertura
y permitirá el encaminamiento de las alertas hasta el gateway/hub IoT (en nuestro
escenario, alejado de los sensores IoT).
* Una vez recibida una notiﬁcación BLE con una alerta de proximidad, el nodo manejará
el evento ESP_GATTC_NOTIFY_EVT para encapsularla y enviarla al nodo Mesh raíz
utilizando la función esp_mesh_send().
```

##  Tareas en ejecución
1. Inicia una tarea FreeRTOS para ` gattc_start()`.
   * Crea y contiene la lógica del cliente GATT.
2. Inicia una tarea FreeRTOS para `wifim_start()`.
   * Crea y contiene la lógica del nodo NO raíz de  Wi-Fi MESH

Las dos tareas comparten el envío de alertas en FreeRTOS utilizando una **Cola de Mensajes (FreeRTOS Queue)**
* La tarea del GATT (Bluetooth) verifica si contiene "prox_alert", enviado por el servidor GATT. Si es así, copia el mensaje a la cola.
* La tarea de transmisión WiFi Mesh se duerma (consumiendo 0 CPU) hasta que reciba datos. Posteriormente lo envia al nodo raíz.

## Directorio del proyecto
A continuación se muestra una explicación de los archivos en la carpeta del proyecto `gattc_wifih`.
```
├── CMakeLists.txt
├── components
│   ├── gattc_comp                      <-- Componente para GATT-CLIENT
│   │   ├── CMakeLists.txt
│   │   ├── gattc_comp.c
│   │   ├── include
│   │   │   └── gattc_comp.h
│   │   └── Kconfig.projbuild
│   └── wifim_comp                      <-- Componente para WIFI-MESH
│       ├── CMakeLists.txt
│       ├── include
│       │   ├── mesh_light.h
│       │   └── wifim_comp.h
│       ├── Kconfig.projbuild
│       ├── mesh_light.c
│       └── wifim_comp.c
├── main
│   ├── CMakeLists.txt
│   └── gattc_wifih.c
├── partitions.csv
├── pytest_hello_world.py
├── README.md
├── sdkconfig
├── sdkconfig.ci
├── sdkconfig.defaults
└── sdkconfig.old
```

## Cómo ejecutar el proyecto
Antes de configurar y construir el proyecto, asegúrese de configurar el chip objetivo correcto utilizando `idf.py set-target <chip_name>`.

### Hardware requerido
* Una placa de desarrollo con ESP32/ESP32-C3 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.).
* Un cable USB para alimentación y programación.

### Configuración del proyecto antes de puesta en marcha
Abrir el menu de configuración del proyecto (`idf.py menuconfig`).

1. En el menu `GATT-CLIENT Configuration  --->`:
* Establecer la configuración de ejemplo.
```
(ESP_GATTS_EJ-1) Nombre del dispositivo BLE                 # El nombre tiene que coincidir con el servidor GATT
()  Remote BLE Device Address (e.g., 34:85:18:02:70:4E)
(1000) BLE scan interval (ms)
(100) BLE scan window (ms)
[ ] Dump whole adv data and scan response data in example
(0) example id for CI test
(0) The pipeline id for CI test
[ ] Perform init deinit of bluedroid host in a loop
```

2. En el menu `GATT-SERVER Configuration  --->`:
* Establecer la configuración de ejemplo.
```
...
(ROUTER_SSID) Router SSID                                       # El router se queda en blanco, ya que este NO será el nodo raíz. 
(ROUTER_PASSWD) Router password                     # Automaticamente creara una Malla Wi-Fi Mesh y buscará al nodo raíz
    Mesh AP Authentication Mode (WIFI_AUTH_WPA2_PSK)  --->
...
```

3. Este ejemplo como utiliza Bluetooth y Wi-Fi. Se encuentra activado por defecto en el archivo `sdkconfig.defaults`
* Señalar que se crea una tabla de particiones personalizada (Recomendado)
```
CONFIG_BT_ENABLED=y
# CONFIG_BT_BLE_50_FEATURES_SUPPORTED is not set
CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y
# CONFIG_BT_LE_50_FEATURE_SUPPORT is not used on ESP32, ESP32-C3 and ESP32-S3.
# CONFIG_BT_LE_50_FEATURE_SUPPORT is not set

# Configuración para usar una tabla de particiones personalizada
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
CONFIG_COMPILER_OPTIMIZATION_SIZE=y
```

### Construir y flashear
Construya el proyecto y fórmelo en la placa, luego ejecute la herramienta de monitorización para ver la salida en serie:
Ejecute `idf.py -p PORT flash monitor` para compilar, actualizar y monitorear el proyecto.
(Para salir del monitor serial, escriba ``Ctrl-]``.)

## Ejemplo de Salida
Se muestra como el componente para GATT-CLIENT envia el payload al componente para WIFI-MESH
```
I (11418) mesh: [scan]new scanning time:600ms, beacon interval:300ms
I (11438) mesh: 2004<arm>parent monitor, my layer:2(cap:6)(node), interval:9327ms, retries:1<normal connected>
I (11438) NODO_MESH: <MESH_EVENT_PARENT_CONNECTED>layer:0-->2, parent:60:55:f9:c1:13:f5<layer2>, ID:77:77:77:77:77:0b, duty:10
I (11448) NODO_MESH: <MESH_EVENT_TODS_REACHABLE>state:0
I (11448) NODO_MESH: Tarea TX iniciada. Esperando datos en 'mesh_tx_queue'...
I (13968) GATT_CLIENT: Notification received
I (13968) GATT_CLIENT: 70 72 6f 78 5f 61 6c 65 72 74 20 f0 9f 91 be
I (13968) GATT_CLIENT: payload: prox_alert 👾
W (13968) GATT_CLIENT: DETECTADO PROX_ALERT 👾 -> Enviando a Mesh...
I (13978) wifi:<ba-add>idx:0 (ifx:0, 60:55:f9:c1:13:f5), tid:5, ssn:0, winSize:64
I (13988) NODO_MESH: HIJO: Mensaje enviado al ROOT: 'prox_alert 👾'
I (16728) GATT_CLIENT: Notification received
I (16728) GATT_CLIENT: 70 72 6f 78 5f 61 6c 65 72 74 20 f0 9f 91 be
I (16728) GATT_CLIENT: payload: prox_alert 👾
W (16728) GATT_CLIENT: DETECTADO PROX_ALERT 👾 -> Enviando a Mesh...
I (16738) NODO_MESH: HIJO: Mensaje enviado al ROOT: 'prox_alert 👾'
I (19838) wifi:pm start, type: 1
```

## Problemas detectados
### El error que estás viendo (reason 0x08) es un "Connection Timeout".
Esto ocurre porque el dispositivo BLE se aleja, hay mucha interferencia o, lo más probable en este caso, el dispositivo BLE (servidor/tag) entra en modo de bajo consumo y cierra la conexión tras enviar la alerta.

Para que el sistema se vuelva a conectar automáticamente y envíe la siguiente alerta, debemos modificar la lógica en gattc_comp.c. Actualmente, el código probablemente deja de escanear una vez que se conecta o se desconecta.

Flujo de operación resultante:
1. Estado Reposo: El ESP32 está escaneando.
2. Detección: El tag BLE aparece y envía publicidad. El ESP32 lo reconoce por el nombre.
3. Acción: Se conecta, recibe la notificación prox_alert 👾, la mete en la cola Mesh y el componente WiFi Mesh la envía.
4. Cierre: El tag se desconecta (tu error 0x08).
5. Reactivación: El evento de desconexión dispara esp_ble_gap_start_scanning(0), volviendo al paso 1.

### Para lograr que el componente BLE (gattc_comp) envíe datos al componente WiFi Mesh (wifim_comp)
Solo cuando ocurre un evento específico (la alerta "prox_alert 👾") y detener el envío constante de datos basura, necesitamos implementar una Cola de Mensajes (FreeRTOS Queue).
1. Eliminado: El bucle while en wifim_comp.c ya no tiene un contador ni envía datos (light_on/light_off) cada X segundos.
2. Agregado: xQueueReceive con portMAX_DELAY hace que la tarea de transmisión WiFi Mesh se duerma (consumiendo 0 CPU) hasta que reciba datos.
3. Puente: Cuando gattc_comp.c recibe ESP_GATTC_NOTIFY_EVT, verifica si contiene "prox_alert". Si es así, copia el mensaje a la cola.
4. Resultado: El log constante desaparece. Solo verás actividad en el log de Mesh cuando escanees una etiqueta BLE que envíe la alerta específica.

### Stack Overflow (desbordamiento de pila)
Tu tarea solo tiene 2044 bytes de stack (aprox. 2KB). Para un proyecto que combina Bluetooth + Wi-Fi Mesh, esto es extremadamente poco. Solo el proceso de autenticación de Wi-Fi (visto en el backtrace con aes_128_cbc, pbkdf2_sha1) consume mucha pila.

Solución:
```
// Cambia esto (probablemente tienes 2048 o menos):
xTaskCreate(wifim_task, "wifim_task", 4096, NULL, 5, NULL);

// Por esto (mínimo recomendado para Mesh + BLE):
xTaskCreate(wifim_task, "wifim_task", 8192, NULL, 5, NULL);
```

### Cuando juntas Bluetooth (BLE) y Wi-Fi Mesh en un mismo proyecto
El error que estás recibiendo es muy común cuando juntas Bluetooth (BLE) y Wi-Fi Mesh en un mismo proyecto de ESP-IDF. El mensaje es claro:
```
Error: app partition is too small [...] size 0x17b460 [...] overflow 0x7b460
```
Tu binario compilado ocupa aproximadamente 1.48 MB (0x17b460), pero tu tabla de particiones por defecto solo reserva 1 MB (0x100000) para la aplicación. Te sobran unos 500 KB.

### ¿Por qué crece tanto el binario?
Al incluir gattc_comp (Bluetooth) y wifim_comp (Mesh), estás compilando dos de los stacks más pesados de Espressif. El stack de Bluetooth por sí solo consume mucho espacio de Flash.

Solución 1: Usar una tabla de particiones personalizada (Recomendado)
Debes crear un archivo llamado partitions.csv en la raíz de tu proyecto (donde está el CMakeLists.txt principal) para ampliar el tamaño de la partición factory.

Crea el archivo `partitions.csv` con este contenido:
```
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x6000,
phy_init, data, phy,     ,        0x1000,
factory,  app,  factory, ,        0x1D0000,
Nota: He puesto 0x1D0000 (1.8 MB), lo cual debería ser suficiente para tu binario actual.
```
Configura el proyecto para que use este archivo:
* Ejecuta `idf.py menuconfig`.
* Ve a `Partition Table -> Partition Table`.
* Selecciona Custom partition table CSV.
* En Custom partition CSV file, asegúrate de que ponga partitions.csv.

###  Modiﬁcáis su código y el de los nodos Mesh regulares para ﬁjar explícitamente cuál será el nodo raíz.
Cabe destacar que en el ejemplo examples/mesh/internal_communica. Donde todos los nodos de la red WIFI Mesh intentarán primero conectarse al punto de acceso y, tras elegirse el nodo raíz, se conectarán directamente entre ellos. Por tanto, modiﬁcáis su código y el de los nodos Mesh regulares para ﬁjar explícitamente cuál será el nodo raíz.

Beneficios de este enfoque:
* Sin errores de sintaxis: Evitas tocar el código fuente cada vez que cambias de dispositivo.
* Persistencia: La configuración se guarda en el archivo sdkconfig de tu proyecto.
* Claridad: Cualquier desarrollador que trabaje en el proyecto verá la opción claramente en el menú de configuración de ESP-IDF.

### Crear el archivo `sdkconfig.defaults`
Para asegurar que una configuración de Kconfig (como habilitar el Bluetooth) se mantenga activa incluso después de un `idf.py fullclean`, la forma correcta y profesional de hacerlo no es mediante el archivo sdkconfig (que se borra con la limpieza), sino a través de un archivo llamado `sdkconfig.defaults`.

