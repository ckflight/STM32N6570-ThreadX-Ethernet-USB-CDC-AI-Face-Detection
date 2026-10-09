import socket
import time

STM32_IP = "10.42.0.158"
PORT = 5000

TOTAL_SIZE = 128 * 1024 * 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((STM32_IP, PORT))

received = 0
start_time = time.perf_counter()


while received < TOTAL_SIZE:
    data = sock.recv(64 * 1024)

    if not data:
        print("Connection closed by STM32")
        break

    received += len(data)
    print(received)

end_time = time.perf_counter()

sock.close()

elapsed = end_time - start_time

print(f"Received   : {received / (1024 * 1024):.2f} MiB")
print(f"Time       : {elapsed:.3f} s")
print(f"Throughput : {received / elapsed / (1024 * 1024):.2f} MiB/s")
print(f"Throughput : {received * 8 / elapsed / 1_000_000:.2f} Mbit/s")