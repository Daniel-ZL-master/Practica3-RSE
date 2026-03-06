import socket
import threading

HOST = '0.0.0.0'
PORT = 8080

# Función que se ejecuta en segundo plano para leer los datos del ESP32
def recibir_datos(conn):
    while True:
        try:
            data = conn.recv(1024)
            if not data:
                print("\n[!] ESP32 desconectado.")
                break
            print(f"ESP32 -> {data.decode('utf-8').strip()}")
        except Exception as e:
            break

print(f"Iniciando servidor en el puerto {PORT}. Esperando al ESP32...")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    
    while True:
        conn, addr = s.accept()
        print(f"\n[+] ESP32 conectado desde la IP: {addr[0]}")
        print(">>> Escribe 'start' para comenzar a recibir la hora, o 'stop' para pausar.\n")
        
        # Iniciamos el hilo secundario para escuchar al ESP32
        hilo_lectura = threading.Thread(target=recibir_datos, args=(conn,), daemon=True)
        hilo_lectura.start()
        
        # Bucle principal (interfaz del usuario) para enviar comandos
        try:
            while True:
                comando = input() # Espera a que escribas algo en la consola
                if comando.lower() in ['start', 'stop']:
                    # Enviamos el comando añadiendo el salto de línea (\n)
                    conn.sendall((comando + '\n').encode('utf-8'))
                else:
                    print("[-] Comando no reconocido. Por favor, escribe 'start' o 'stop'.")
        except KeyboardInterrupt:
            print("\nCerrando conexión...")
            conn.close()
            break
