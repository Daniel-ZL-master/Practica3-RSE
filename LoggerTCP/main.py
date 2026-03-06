# This is a sample Python script.

# Press Mayús+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import csv
from datetime import datetime
import socket
import threading

HOST = '0.0.0.0'
PORT = 8080
log_filename = f"log_practica5_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"


# Función que se ejecuta en segundo plano para leer los datos del ESP32
def recibir_datos(conn):
    while True:
        try:
            data = conn.recv(1024)
            if not data:
                print("\n[!] ESP32 desconectado.")
                break
            message = data.decode('utf-8').strip()
            output_data = message.split(';')
            with open(log_filename, mode='a') as csvfile:
                logger = csv.writer(csvfile)
                logger.writerow(output_data)
        except Exception as e:
            break

if __name__ == "__main__":
    print("Creando log...")
    log_filename = f"log_practica5_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
    with open(log_filename, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(
            ['Aceleracion X', 'Aceleracion Y', 'Aceleracion Z'])
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
                    comando = input()  # Espera a que escribas algo en la consola
                    if comando.lower() in ['start', 'stop']:
                        # Enviamos el comando añadiendo el salto de línea (\n)
                        conn.sendall((comando + '\n').encode('utf-8'))
                    else:
                        print("[-] Comando no reconocido. Por favor, escribe 'start' o 'stop'.")
            except KeyboardInterrupt:
                print("\nCerrando conexión...")
                conn.close()
                break