
import serial
import struct
from pathlib import Path

PORT = "/dev/ttyACM1"
IMAGE_SIZE = 112 * 112 * 3

# Save next to record_ai.py
BASE_DIR = Path(__file__).resolve().parent
OUTPUT_PATH = BASE_DIR / "cenk_input.bin"


def read_exact(ser, count):
    data = bytearray()

    while len(data) < count:
        chunk = ser.read(count - len(data))

        if not chunk:
            raise TimeoutError(
                f"Expected {count}, received {len(data)} bytes"
            )

        data.extend(chunk)

    return bytes(data)


with serial.Serial(PORT, 115200, timeout=15) as ser:

    ser.reset_input_buffer()

    # Request capture from STM32
    ser.write(b"START")
    ser.flush()

    print("START sent. Waiting for CENK packet...")

    marker = bytearray()

    while True:
        byte = ser.read(1)

        if not byte:
            raise TimeoutError("CENK packet not received")

        marker.extend(byte)
        marker = marker[-4:]

        if marker == b"CENK":
            break

    # Read input size
    size = struct.unpack("<I", read_exact(ser, 4))[0]
    print("Input size:", size)

    if size != IMAGE_SIZE:
        raise ValueError(f"Unexpected size: {size}")

    # Read NN input and raw output
    image = read_exact(ser, size)
    raw = read_exact(ser, 1)[0]

    # Save exact NN input bytes
    with open(OUTPUT_PATH, "wb") as f:
        f.write(image)

    print("Saved:", OUTPUT_PATH)
    print("STM32 Raw:", raw)
    print("STM32 Score:", raw / 256.0)
