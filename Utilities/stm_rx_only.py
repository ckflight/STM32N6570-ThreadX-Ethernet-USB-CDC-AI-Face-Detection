import socket
import time
import numpy as np

STM32_IP = "10.42.0.158"
PORT = 5000

CHUNK_SIZE = 64 * 1024
CHUNK_COUNT = 4096

tx_data = np.random.randint(0, 256, CHUNK_SIZE, dtype=np.uint8).tobytes()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((STM32_IP, PORT))

total_tx_size = CHUNK_COUNT * CHUNK_SIZE

print(f"Chunk size : {CHUNK_SIZE} bytes")
print(f"Total size : {total_tx_size / (1024 * 1024):.2f} MiB")

start_time = time.perf_counter()

for i in range(CHUNK_COUNT):
    sock.sendall(tx_data)

end_time = time.perf_counter()

sock.close()

total_time = end_time - start_time

throughput_mib = total_tx_size / total_time / (1024 * 1024)
throughput_mbit = total_tx_size * 8 / total_time / 1_000_000

print(f"Time       : {total_time:.3f} s")
print(f"Throughput : {throughput_mib:.2f} MiB/s")
print(f"Throughput : {throughput_mbit:.2f} Mbit/s")