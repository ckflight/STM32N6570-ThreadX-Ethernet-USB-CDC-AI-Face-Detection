
import tensorflow as tf

MODEL_PATH = "cenk_classifier.keras"
DATASET_PATH = "dataset"
OUTPUT_PATH = "cenk_classifier_int8.tflite"

model = tf.keras.models.load_model(MODEL_PATH)

calib_ds = tf.keras.utils.image_dataset_from_directory(
    DATASET_PATH,
    image_size=(112, 112),
    batch_size=1,
    shuffle=True,
    seed=42
)

def representative_dataset():
    for images, _ in calib_ds.take(200):
        yield [tf.cast(images, tf.float32)]

converter = tf.lite.TFLiteConverter.from_keras_model(model)

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.uint8
converter.inference_output_type = tf.uint8

tflite_model = converter.convert()

with open(OUTPUT_PATH, "wb") as f:
    f.write(tflite_model)

print("Saved:", OUTPUT_PATH)

interpreter = tf.lite.Interpreter(model_path=OUTPUT_PATH)
interpreter.allocate_tensors()

inp = interpreter.get_input_details()[0]
out = interpreter.get_output_details()[0]

print("INPUT:", inp["shape"], inp["dtype"], inp["quantization"])
print("OUTPUT:", out["shape"], out["dtype"], out["quantization"])
