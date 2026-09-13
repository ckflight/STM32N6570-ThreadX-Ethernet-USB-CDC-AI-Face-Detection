import serial
import numpy as np
import time

PORT = "/dev/ttyACM1"
READ_SIZE = 4 * 1024 * 1024   # 4 MB

ser = serial.Serial(PORT, timeout=2)

expected_offset = 0
total_bytes = 0
total_errors = 0
start = time.time()

while True:
    data = ser.read(READ_SIZE)

    if not data:
        continue

    rx = np.frombuffer(data, dtype=np.uint8)

    expected = (
        np.arange(expected_offset,
                  expected_offset + len(rx),
                  dtype=np.uint32) & 0xFF
    ).astype(np.uint8)

    errors = np.count_nonzero(rx != expected)

    if errors:
        idx = np.flatnonzero(rx != expected)[0]

        print(
            f"\nERROR at byte {total_bytes + idx}: "
            f"RX=0x{rx[idx]:02X}, "
            f"EXPECTED=0x{expected[idx]:02X}"
        )

    total_errors += errors
    total_bytes += len(rx)
    expected_offset = (expected_offset + len(rx)) & 0xFF

    elapsed = time.time() - start
    speed = total_bytes / elapsed / 1e6

    print(
        f"\rReceived: {total_bytes / 1e6:8.1f} MB | "
        f"Speed: {speed:5.1f} MB/s | "
        f"Errors: {total_errors}",
        end="",
        flush=True
    )