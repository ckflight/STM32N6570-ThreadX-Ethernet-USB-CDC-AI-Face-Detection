import socket
import serial
import numpy as np
import time
import threading

STM32_IP = "10.42.0.158"
TCP_PORT = 5000
SERIAL_PORT = "/dev/ttyACM1"

ETH_TOTAL_SIZE = 64 * 1024 * 1024
USB_READ_SIZE = 1 * 1024 * 1024

start_event = threading.Event()
stop_event = threading.Event()

eth_bytes = 0
usb_bytes = 0
usb_errors = 0

eth_done = False


def ethernet_test():
    global eth_bytes, eth_done

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((STM32_IP, TCP_PORT))

    print("Ethernet connected")

    start_event.wait()

    while eth_bytes < ETH_TOTAL_SIZE:
        data = sock.recv(64 * 1024)

        if not data:
            break

        eth_bytes += len(data)

    sock.close()

    eth_done = True
    stop_event.set()


def usb_test():
    global usb_bytes, usb_errors

    ser = serial.Serial(SERIAL_PORT, timeout=0.2)

    print("USB connected")

    expected_offset = 0

    start_event.wait()

    while not stop_event.is_set():
        data = ser.read(USB_READ_SIZE)

        if not data:
            continue

        rx = np.frombuffer(data, dtype=np.uint8)

        expected = (
            np.arange(
                expected_offset,
                expected_offset + len(rx),
                dtype=np.uint32
            ) & 0xFF
        ).astype(np.uint8)

        errors = np.count_nonzero(rx != expected)

        usb_errors += errors
        usb_bytes += len(rx)

        expected_offset = (expected_offset + len(rx)) & 0xFF

    ser.close()


eth_thread = threading.Thread(target=ethernet_test)
usb_thread = threading.Thread(target=usb_test)

eth_thread.start()
usb_thread.start()

time.sleep(1)

print("START BOTH\n")

start_event.set()

start = time.perf_counter()

last_time = start
last_eth = 0
last_usb = 0

while not stop_event.is_set():

    time.sleep(1)

    now = time.perf_counter()
    dt = now - last_time

    eth_speed = (eth_bytes - last_eth) / dt / 1e6
    usb_speed = (usb_bytes - last_usb) / dt / 1e6

    total_speed = eth_speed + usb_speed

    print(
        f"ETH: {eth_speed:6.2f} MB/s | "
        f"USB: {usb_speed:6.2f} MB/s | "
        f"TOTAL: {total_speed:6.2f} MB/s | "
        f"USB Errors: {usb_errors}"
    )

    last_eth = eth_bytes
    last_usb = usb_bytes
    last_time = now

eth_thread.join()
usb_thread.join()

elapsed = time.perf_counter() - start

print("\n========== FINAL ==========")

print(f"Ethernet : {eth_bytes / elapsed / 1e6:.2f} MB/s")
print(f"USB      : {usb_bytes / elapsed / 1e6:.2f} MB/s")
print(f"Combined : {(eth_bytes + usb_bytes) / elapsed / 1e6:.2f} MB/s")
print(f"USB Errors: {usb_errors}")