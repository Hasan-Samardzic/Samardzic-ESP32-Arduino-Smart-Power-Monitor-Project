#=================================
# Hasan Samardzic 
# TCP Client Program CS528 Final
# Wireless Smart Power Usage Meter 
#=================================

import socket

# === Server Settings ===
server_ip = '0.0.0.0'   
server_port = 12345

# === Create TCP Server Socket ===
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind((server_ip, server_port))
server_socket.listen(5)

print(f"[INFO] Listening for ESP32 connections on {server_ip}:{server_port}")

try:
    while True:
        client_socket, client_address = server_socket.accept()
        print(f"[+] Connection from {client_address} established")

        try:
            while True:
                data = client_socket.recv(1024)  # Max 1024 bytes
                if not data:
                    print(f"[-] Connection with {client_address} closed")
                    break
                decoded_data = data.decode('utf-8').strip()
                print(f"[Data Received]: {decoded_data}")
        except Exception as e:
            print(f"[!] Error during client communication: {e}")
        finally:
            client_socket.close()
except KeyboardInterrupt:
    print("\n[INFO] Server shutting down by user interrupt.")
finally:
    server_socket.close()
