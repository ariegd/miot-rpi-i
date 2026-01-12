| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# Objetivos de este Nodo
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

## Nodo gattc_wifi
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

