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
