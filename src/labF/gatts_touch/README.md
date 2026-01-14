| Supported Targets | ESP32 | ESP32-C3  | Linux |
| ----------------- | ----- | -------- | ----- |

# Objetivo del nodo `gatts_touch`
```
2. Implementación del sensor IoT

* El sensor IoT debe medir la proximidad (es decir, detectar si se toca un pin concreto)
utilizando el touch pad capacitivo del ESP32. Para ello, se deben emplear las funciones
touch_pad_init(), touch_pad_conﬁg() y touch_pad_read().
Nota: estableced un umbral de tiempo para controlar el envío de alertas de proximidad
y evitar falsos positivos (p.e. que la alerta sólo se envíe tras tocar el pin durante 1.5
segundos).
* En el servidor GATT se debe registrar un servicio y una característica con su
correspondiente descriptor CCC que permita al cliente GATT suscribirse y recibir
notiﬁcaciones cada vez que se genere una alerta de proximidad. Hasta este punto,
podéis comprobar la comunicación con el servidor GATT usando la aplicación móvil
LightBlue.
Nota: durante todo el proceso de envío de alertas desde los sensores IoT hasta el
gateway/hub IoT, el payload intercambiado puede ser simplemente un valor numérico,
booleano o string corto (p.e. “1”, “true”, “prox_alert”). No es necesario serializar los
datos con JSON o CBOR.
```

##  Tareas en ejecución
1. Inicia una tarea FreeRTOS para ` gatts_start()`.
   * Crea y contiene la lógica del servidor GATT.
2. Inicia una tarea FreeRTOS para `touch_pad_start()`.
   * Crea y contiene la lógica del sensor IoT `touch_pad_*`

Las dos tareas comparten el envío de alertas en FreeRTOS utilizando un **Semáforo Binario**.
* La tarea del GATT (Bluetooth) se quedará "dormida" esperando el semáforo.
* La tarea del Touch "dará" (activará) el semáforo cuando detecte la pulsación larga.

## Directorio del proyecto
A continuación se muestra una explicación de los archivos en la carpeta del proyecto `gatts_touch`.

```
├── CMakeLists.txt
├── components
│   ├── gatts_comp                      <-- Componente para GATT-SERVER
│   │   ├── CMakeLists.txt
│   │   ├── gatts_comp.c
│   │   ├── include
│   │   │   └── gatts_comp.h
│   │   └── Kconfig.projbuild
│   └── touch_pad_comp          <-- Componente para sensor TOUCH-PAD
│       ├── CMakeLists.txt
│       ├── include
│       │   └── touch_pad_comp.h
│       ├── Kconfig.projbuild
│       └── touch_pad_comp.c
├── img
│   └── Habilitar BLE.png
├── main
│   ├── CMakeLists.txt
│   └── gatts_touch.c
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

1. En el menu `TOUCH-PAD Configuration  --->`:
* Establecer la configuración de ejemplo.
```
(2) Numero de Touch Pad (GPIO)
(80) Factor de Umbral (%)
(1500) Tiempo de activacion requerido (ms)
```

2. En el menu `GATT-SERVER Configuration  --->`:
* Establecer la configuración de ejemplo.
```
(ESP_GATTS_EJ-1) Nombre del dispositivo BLE  # El nombre tiene que coincidir con el cliente GATT
```

3. Este ejemplo como utiliza Bluetooth. Se encuentra activado por defecto en el archivo `sdkconfig.defaults`
```
CONFIG_BT_ENABLED=y
# CONFIG_BT_BLE_50_FEATURES_SUPPORTED is not set
CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y
# CONFIG_BT_LE_50_FEATURE_SUPPORT is not used on ESP32, ESP32-C3 and ESP32-S3.
# CONFIG_BT_LE_50_FEATURE_SUPPORT is not set
```

### Construir y flashear
Construya el proyecto y fórmelo en la placa, luego ejecute la herramienta de monitorización para ver la salida en serie:
Ejecute `idf.py -p PORT flash monitor` para compilar, actualizar y monitorear el proyecto.
(Para salir del monitor serial, escriba ``Ctrl-]``.)

## Ejemplo de Salida
Se muestra como el componente para sensor TOUCH-PAD  envia el payload al componente para GATT-SERVER
```
 (13166) GATTS_TABLE_DEMO: notify enable
W (19646) touch_pad_comp: ALERTA: Proximidad detectada por 1.5 segundos!
I (19646) GATT_COMP: Evento recibido. Enviando payload: prox_alert 👾
I (19646) GATTS_TABLE_DEMO: ESP_GATTS_CONF_EVT, status = 0, attr_handle 42
W (25226) touch_pad_comp: ALERTA: Proximidad detectada por 1.5 segundos!
I (25226) GATT_COMP: Evento recibido. Enviando payload: prox_alert 👾
I (25226) GATTS_TABLE_DEMO: ESP_GATTS_CONF_EVT, status = 0, attr_handle 42
W (30986) touch_pad_comp: ALERTA: Proximidad detectada por 1.5 segundos!
I (30986) GATT_COMP: Evento recibido. Enviando payload: prox_alert 👾
I (30986) GATTS_TABLE_DEMO: ESP_GATTS_CONF_EVT, status = 0, attr_handle 42
```

## Escaneo de dispositivos y conexión/manipulación de atributos GATT.
```
sudo hcitool lescan
```
* Salida:
```
LE Scan ...
38:D5:6F:26:17:D0 (unknown)
C4:DD:57:5B:F9:FE ESP_GATTS_EJ-1
C4:DD:57:5B:F9:FE (unknown)
F8:04:2E:AF:3C:EB (unknown)
B0:E4:5C:18:7A:E1 (unknown)
B0:E4:5C:18:7A:E1 65" QLED
E8:AA:CB:F6:01:4B (unknown)
E8:AA:CB:F6:01:4B (unknown)
74:3A:BD:85:AB:EC (unknown)
67:FB:D5:10:09:41 (unknown)
60:8F:27:69:DF:8F (unknown)
29:80:52:91:95:4D (unknown)
29:80:52:91:95:4D (unknown)
47:7A:67:22:45:18 (unknown)
47:7A:67:22:45:18 (unknown)
C4:DD:57:5B:F9:FE ESP_GATTS_EJ-1
```

## La herramienta en modo interactivo
Para comenzar una sesión gatttool, invocaremos a la herramienta en modo interactivo,
utilizando la orden:

```
gatttool -b c4:dd:57:5b:f9:fe -I
[c4:dd:57:5b:f9:fe][LE]> connect
Attempting to connect to c4:dd:57:5b:f9:fe
Connection successful
[c4:dd:57:5b:f9:fe][LE]> char-write-req 0x002b 0100
Characteristic value was written successfully
Notification handle = 0x002a value: 70 72 6f 78 5f 61 6c 65 72 74 20 f0 9f 91 be 
Notification handle = 0x002a value: 70 72 6f 78 5f 61 6c 65 72 74 20 f0 9f 91 be 
Notification handle = 0x002a value: 70 72 6f 78 5f 61 6c 65 72 74 20 f0 9f 91 be 
[c4:dd:57:5b:f9:fe][LE]> exit
```

## Problemas y soluciones
### Envíe la notificación solo cuando el sensor táctil detecta la proximidad (1.5s)
Para lograr que el servidor GATT envíe la notificación solo cuando el sensor táctil detecta la proximidad (1.5s), necesitamos un mecanismo de comunicación entre las dos tareas (la del Touch y la del Bluetooth).

La mejor forma de hacer esto en FreeRTOS es utilizando un Semáforo Binario.
* La tarea del GATT (Bluetooth) se quedará "dormida" esperando el semáforo.
* La tarea del Touch "dará" (activará) el semáforo cuando detecte la pulsación larga.

### Causa del Error
* El error `E (886) FreeRTOS: FreeRTOS Task "gatt_server_tas" should not return, Aborting now!` es muy específico y común cuando se migra código desde app_main a una tarea (Task) de FreeRTOS.
* En FreeRTOS, una tarea nunca debe llegar al final de su función (es decir, no puede hacer return ni cerrar la llave }). A diferencia de las funciones normales, una tarea debe ser un bucle infinito (while(1)) o, si su trabajo termina (como una inicialización), debe autodestruirse explícitamente.
* Solución: Tienes que editar el archivo components/gatt_server_comp/gatt_server_comp.c. Busca la función que se ejecuta como tarea (la que pasaste a xTaskCreate) y añade vTaskDelete(NULL); al final.

### Habilitar Bluetooth en el menuconfig
Si el compilador no encuentra el archivo, puede ser que el componente de Bluetooth esté excluido de la compilación. Asegúrate de que esté activo:
1. Ejecuta: `idf.py menuconfig`
2. Ve a: `Component config -> Bluetooth`
3. Marca la casilla: `[*] Bluetooth`
4. Dentro de Bluetooth, asegúrate de que el "Host" esté configurado (usualmente Bluedroid o NimBLE, según tu código utiliza esp_bluedroid_init).

### Cambios Clave Realizados
* Lógica de Temporización: Se han añadido las variables start_tick y is_touching. Cuando se detecta un toque, se guarda el "tiempo actual". En cada ciclo subsiguiente, se compara el tiempo actual con el inicial.
* Filtro de Falsos Positivos: Si el usuario suelta el sensor antes de los 1.5 segundos (por ejemplo, ruido o un toque accidental), el bloque else resetea la variable is_touching y el contador vuelve a cero.
* Prevención de Repetición: La bandera alert_sent asegura que la alerta (ESP_LOGW) se imprima una sola vez cuando se cumple el tiempo, en lugar de imprimirse continuamente mientras se mantiene el dedo.
