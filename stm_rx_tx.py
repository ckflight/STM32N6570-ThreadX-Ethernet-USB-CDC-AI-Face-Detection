import socket
import threading
import time
import numpy as np

STM32_IP = "10.42.0.158"
PORT = 5000

TEST_TIME = 10
CHUNK_SIZE = 64 * 1024

tx_data = np.random.randint(
    0, 256, CHUNK_SIZE, dtype=np.uint8
).tobytes()

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((STM32_IP, PORT))

tx_total = 0
rx_total = 0
tx_counter = 0
rx_counter = 0
running = True


def tx_thread():
    global tx_total, tx_counter

    while running:
        try:
            sock.sendall(tx_data)

            tx_total += len(tx_data)
            tx_counter += 1

        except Exception:
            break


def rx_thread():
    global rx_total, rx_counter

    while running:
        try:
            data = sock.recv(CHUNK_SIZE)

            if not data:
                break

            rx_total += len(data)
            rx_counter += 1

            print("Rx counter:",rx_counter)

        except Exception:
            break


tx = threading.Thread(target=tx_thread)
rx = threading.Thread(target=rx_thread)

start = time.perf_counter()

tx.start()
rx.start()

time.sleep(TEST_TIME)

running = False

try:
    sock.shutdown(socket.SHUT_RDWR)
except Exception:
    pass

sock.close()

tx.join()
rx.join()

elapsed = time.perf_counter() - start

pc_to_stm32 = tx_total / elapsed / 1_000_000
stm32_to_pc = rx_total / elapsed / 1_000_000

print()
print("Test time       :", elapsed, "s")
print("PC -> STM32 RX  :", pc_to_stm32, "MB/s")
print("STM32 -> PC TX  :", stm32_to_pc, "MB/s")
print("TOTAL           :", pc_to_stm32 + stm32_to_pc, "MB/s")
print()
print("TX bytes        :", tx_total)
print("RX bytes        :", rx_total)
print("TX sendall count:", tx_counter)
print("RX recv count   :", rx_counter)