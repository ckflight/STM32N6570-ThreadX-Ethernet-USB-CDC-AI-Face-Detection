
import sys
import cv2
import numpy as np
import tensorflow as tf

IMAGE_PATH = sys.argv[1] if len(sys.argv) > 1 else "test.jpg"
THRESHOLD = 0.80

img = cv2.imread(IMAGE_PATH)

if img is None:
    raise FileNotFoundError(IMAGE_PATH)

img = cv2.resize(img, (112, 112))
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

# Keras model: float32 input, pixel range 0..255
keras_model = tf.keras.models.load_model("cenk_classifier.keras")
keras_input = np.expand_dims(img, axis=0).astype(np.float32)

keras_score = float(keras_model.predict(keras_input, verbose=0)[0][0])

# TFLite model: uint8 input, pixel range 0..255
interpreter = tf.lite.Interpreter(model_path="cenk_classifier_int8.tflite")
interpreter.allocate_tensors()

inp = interpreter.get_input_details()[0]
out = interpreter.get_output_details()[0]

tflite_input = np.expand_dims(img, axis=0).astype(np.uint8)

interpreter.set_tensor(inp["index"], tflite_input)
interpreter.invoke()

raw = int(interpreter.get_tensor(out["index"])[0][0])
scale, zero_point = out["quantization"]
tflite_score = (raw - zero_point) * scale

print("INPUT:", inp["dtype"], inp["quantization"])
print("OUTPUT:", out["dtype"], out["quantization"])

print(f"KERAS SCORE: {keras_score:.4f}")
print(f"TFLITE RAW: {raw}")
print(f"TFLITE SCORE: {tflite_score:.4f}")

print("KERAS:", "CENK" if keras_score >= THRESHOLD else "NOT CENK")
print("TFLITE:", "CENK" if tflite_score >= THRESHOLD else "NOT CENK")
