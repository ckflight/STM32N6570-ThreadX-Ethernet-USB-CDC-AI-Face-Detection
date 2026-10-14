
from pathlib import Path
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt

# Directories
BASE_DIR = Path(__file__).resolve().parent
MODEL_DIR = BASE_DIR.parent / "Face_Recognition_Model"

# Files
MODEL_PATH = MODEL_DIR / "cenk_classifier_int8.tflite"
INPUT_PATH = BASE_DIR / "cenk_input.bin"

# Load TFLite model
interpreter = tf.lite.Interpreter(model_path=str(MODEL_PATH))
interpreter.allocate_tensors()

input_info = interpreter.get_input_details()[0]
output_info = interpreter.get_output_details()[0]

print("Input:", input_info["shape"], input_info["dtype"])
print("Output:", output_info["shape"], output_info["dtype"])

# Load recorded STM32 NN input
image = np.fromfile(INPUT_PATH, dtype=np.uint8)

expected_size = int(np.prod(input_info["shape"]))

if image.size != expected_size:
    raise ValueError(
        f"Input size mismatch: expected {expected_size}, got {image.size}"
    )

if input_info["dtype"] != np.uint8:
    raise TypeError(f"Unsupported input dtype: {input_info['dtype']}")

image = image.reshape(input_info["shape"])

# Run TFLite inference
interpreter.set_tensor(input_info["index"], image)
interpreter.invoke()

# Read output
output = interpreter.get_tensor(output_info["index"])
raw = int(output.flat[0])

# Dequantize output
scale, zero_point = output_info["quantization"]

print("\n--- TFLITE RESULTS ---")
print("Python Raw:", raw)

if scale > 0:
    score = (raw - zero_point) * scale
    print("Python Score:", score)
else:
    score = None
    print("Python Output:", output)

# Plot exact NN input image
plt.figure(figsize=(5, 5))
plt.imshow(image[0])
plt.title(
    f"TFLite Input (112x112 RGB)\n"
    f"Raw: {raw} | Score: {score:.6f}"
    if score is not None
    else f"TFLite Input (112x112 RGB)\nRaw: {raw}"
)
plt.axis("off")
plt.tight_layout()
plt.show()
