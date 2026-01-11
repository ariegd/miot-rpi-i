| Supported Targets | ESP32 | ESP32-C3  | Linux |
| ----------------- | ----- | -------- | ----- |

#  Ejemplo de Touch Pad

Inicia una tarea FreeRTOS para imprimir "touch_example: value=682, touched=0".

## Ejemplo del directorio del proyecto

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

## Ejemplo de Salida

This example's output maybe could not give a strong feeling to user since the waterproof function works
automatically and silently inside the Touch Element library

```
I (17678) touch_example: value=61, touched=1
I (17788) touch_example: value=53, touched=1
I (17898) touch_example: value=51, touched=1
I (18008) touch_example: value=51, touched=1
I (18118) touch_example: value=54, touched=1
I (18228) touch_example: value=671, touched=0
I (18338) touch_example: value=682, touched=0
I (18448) touch_example: value=637, touched=0
I (18558) touch_example: value=568, touched=0
I (18668) touch_example: value=174, touched=1
I (18778) touch_example: value=128, touched=1
I (18888) touch_example: value=121, touched=1
I (18998) touch_example: value=118, touched=1
I (19108) touch_example: value=180, touched=1
I (19218) touch_example: value=682, touched=0
I (19328) touch_example: value=682, touched=0
I (19438) touch_example: value=680, touched=0
I (19548) touch_example: value=123, touched=1
```

2. Implementación del sensor IoT
El sensor IoT debe medir la proximidad (es decir, detectar si se toca un pin concreto)
utilizando el touch pad capacitivo del ESP32. Para ello, se deben emplear las funciones
`touch_pad_init()`, `touch_pad_conﬁg()` y `touch_pad_read()`.

```
detector de proximidad           -->            nodo WiFi Mesh regular          -->         nodo WiFi Mesh raíz      -->        gateway/hub (PC servidor CoAP)
y servidor GATT                   (GATT)                   y cliente GATT            (Wi-Fi Mesh)                                       (Wi-Fi y CoAP)
```

## Problemas y soluciones

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
