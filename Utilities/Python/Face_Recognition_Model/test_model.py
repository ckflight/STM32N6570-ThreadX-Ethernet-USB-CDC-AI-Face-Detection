
import os
os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

import cv2
import numpy as np
import tensorflow as tf
from pathlib import Path

IMAGE_DIR = Path("/home/ck/Downloads/dataset/cenk")
MODEL_DIR = Path("/home/ck/Desktop/Workspace/STM32CubeIDE Workspace/STM32N6570-ThreadX-USB-CDC-AI-Face-Detection-Ethernet/Utilities/Python/Face_Recognition_Model")
THRESHOLD = 0.80

# Load models once
keras_model = tf.keras.models.load_model(MODEL_DIR / "cenk_classifier.keras", compile=False)

interpreter = tf.lite.Interpreter(model_path=str(MODEL_DIR / "cenk_classifier_int8.tflite"))
interpreter.allocate_tensors()

inp = interpreter.get_input_details()[0]
out = interpreter.get_output_details()[0]
scale, zero_point = out["quantization"]

print("INPUT:", inp["dtype"], inp["quantization"])
print("OUTPUT:", out["dtype"], out["quantization"])
print("-" * 100)

keras_correct = 0
tflite_correct = 0
total = 0

image_paths = sorted(p for p in IMAGE_DIR.iterdir() if p.suffix.lower() in [".jpg", ".jpeg", ".png"])

for path in image_paths:
    img = cv2.imread(str(path))

    if img is None:
        print("Cannot read:", path.name)
        continue

    img = cv2.resize(img, (112, 112))
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # Keras inference
    keras_input = np.expand_dims(img, axis=0).astype(np.float32)
    keras_score = float(keras_model(keras_input, training=False).numpy()[0][0])

    # Quantized TFLite inference
    tflite_input = np.expand_dims(img, axis=0).astype(np.uint8)
    interpreter.set_tensor(inp["index"], tflite_input)
    interpreter.invoke()

    raw = int(interpreter.get_tensor(out["index"])[0][0])
    tflite_score = (raw - zero_point) * scale

    keras_prediction = "CENK" if keras_score >= THRESHOLD else "NOT CENK"
    tflite_prediction = "CENK" if tflite_score >= THRESHOLD else "NOT CENK"

    if keras_prediction == "CENK":
        keras_correct += 1

    if tflite_prediction == "CENK":
        tflite_correct += 1

    total += 1

    print(f"{path.name:<30} KERAS: {keras_score:.4f} ({keras_prediction:<8}) | TFLITE: {tflite_score:.4f} ({tflite_prediction:<8}) | RAW: {raw}")

print("-" * 100)
print(f"TOTAL IMAGES   : {total}")

if total > 0:
    print(f"KERAS ACCURACY : {keras_correct}/{total} ({keras_correct / total * 100:.2f}%)")
    print(f"TFLITE ACCURACY: {tflite_correct}/{total} ({tflite_correct / total * 100:.2f}%)")
