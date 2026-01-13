Entiendo perfectamente. El código que compartes es el ejemplo clásico de comunicación **P2P (Peer-to-Peer)** dentro de una red ESP-WIFI-MESH.

Aquí tienes la explicación de dónde ocurre la magia y cómo detener ese tráfico constante.

## ---

**1\. ¿Dónde se envía y recibe la información?**

El flujo de datos está dividido en dos tareas principales de FreeRTOS que se inician en la función esp\_mesh\_comm\_p2p\_start():

### **El Envío (Transmisión)**

Se gestiona en la función **esp\_mesh\_p2p\_tx\_main** (Línea 65).

* **Quién envía:** Solo el nodo **ROOT** (Raíz) envía estos paquetes masivos. El código tiene un condicional: if (\!esp\_mesh\_is\_root()) { continue; }.  
* A quién: El Root obtiene la tabla de rutas (esp\_mesh\_get\_routing\_table) y recorre todos los nodos conectados para enviarles un paquete individual (Unicast) usando la función:  
  esp\_mesh\_send(\&route\_table\[i\], \&data, MESH\_DATA\_P2P, NULL, 0);  
* **Qué envía:** Envía un buffer de 1460 bytes (TX\_SIZE) que contiene comandos para encender o apagar un LED (light\_on / light\_off).

### **La Recepción**

Se gestiona en la función **esp\_mesh\_p2p\_rx\_main** (Línea 116).

* Cómo recibe: Todos los nodos (incluidos los hijos) están bloqueados en la función:  
  esp\_mesh\_recv(\&from, \&data, portMAX\_DELAY, \&flag, NULL, 0);  
* **Procesamiento:** Una vez llega un paquete, el código extrae el contador y ejecuta mesh\_light\_process para actuar sobre el hardware (el LED). Los mensajes que ves en tu consola provienen del ESP\_LOGW que está justo después de recibir el dato.

## ---

**2\. Cómo eliminar el envío constante de paquetes**

Los logs que recibes (\#RX:41/41...) confirman que tu nodo está recibiendo ráfagas de datos de 1460 bytes constantemente. Para detener esto, tienes tres opciones dependiendo de lo que busques:

### **Opción A: Desactivar la comunicación de prueba por completo**

Si quieres usar el código como base pero no quieres que envíe nada automáticamente, ve a la función esp\_mesh\_comm\_p2p\_start y comenta las líneas donde se crean las tareas:

C

esp\_err\_t esp\_mesh\_comm\_p2p\_start(void)  
{  
    static bool is\_comm\_p2p\_started \= false;  
    if (\!is\_comm\_p2p\_started) {  
        is\_comm\_p2p\_started \= true;  
        // Comenta estas dos líneas:  
        // xTaskCreate(esp\_mesh\_p2p\_tx\_main, "MPTX", 3072, NULL, 5, NULL);  
        // xTaskCreate(esp\_mesh\_p2p\_rx\_main, "MPRX", 3072, NULL, 5, NULL);  
    }  
    return ESP\_OK;  
}

### **Opción B: Eliminar solo los logs de recepción (Silenciar la consola)**

Si quieres que la comunicación siga funcionando (que el Root controle a los nodos) pero no quieres ver los mensajes amarillos de advertencia en la consola, elimina o comenta el ESP\_LOGW dentro de **esp\_mesh\_p2p\_rx\_main**:

C

// Alrededor de la línea 146, comenta este bloque:  
/\*  
if (\!(recv\_count % 1)) {  
    ESP\_LOGW(MESH\_TAG, "\[\#RX:%d/%d\]...", ...);  
}  
\*/

### **Opción C: Cambiar la lógica del Root**

Si quieres que el Root deje de inundar la red, debes modificar el bucle while(is\_running) dentro de **esp\_mesh\_p2p\_tx\_main**. Actualmente, no tiene casi ningún retardo si hay muchos nodos, lo que satura el ancho de banda. Puedes aumentar el vTaskDelay al final del bucle de envío para que solo envíe un paquete cada 30 segundos, por ejemplo.