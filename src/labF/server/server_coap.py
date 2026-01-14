import asyncio
import logging
import aiocoap.resource as resource
import aiocoap

# Log para ver los paquetes UDP crudos que llegan
logging.basicConfig(level=logging.INFO)

class MasterResource(resource.Resource):
    """Recurso que acepta TODO"""
    def __init__(self):
        super().__init__()

    async def needs_blockwise(self, request):
        return False

    # Esta función captura TODO lo que no coincide con otras rutas
    async def render(self, request):
        payload = request.payload.decode('utf-8', errors='ignore')
        method = request.code
        # Esto nos dirá qué ruta está pidiendo el ESP32 realmente
        path = request.opt.uri_path
        
        print("\n" + "!"*40)
        print(f"📥 PETICIÓN CAPTURADA")
        print(f"   Método: {method}")
        print(f"   Ruta (Path): {path}")
        print(f"   Payload: {payload}")
        print(f"   Desde: {request.remote.hostinfo}")
        print("!"*40 + "\n")
        
        return aiocoap.Message(code=aiocoap.CHANGED, payload=b"Capturado")

async def main():
    # Creamos el sitio SIN añadir recursos específicos al principio
    root = resource.Site()
    
    # El truco: Añadimos un recurso que captura todo lo que empiece por 'Espressif'
    # o simplemente lo ponemos como recurso raíz si es necesario
    master = MasterResource()
    root.add_resource(['Espressif'], master)
    
    # Intentamos también con minúsculas por si acaso
    root.add_resource(['espressif'], master)

    await aiocoap.Context.create_server_context(root, bind=('0.0.0.0', 5683))
    
    print("🚀 Servidor de Diagnóstico Iniciado")
    print("Esperando cualquier mensaje en /Espressif o /espressif...")
    
    await asyncio.get_running_loop().create_future()

if __name__ == "__main__":
    asyncio.run(main())
