
import tensorflow as tf
import numpy as np

DATASET_PATH = "dataset"
IMG_SIZE = (112, 112)
BATCH_SIZE = 32
EPOCHS = 30
SEED = 42

tf.keras.utils.set_random_seed(SEED)

train_ds = tf.keras.utils.image_dataset_from_directory(
    DATASET_PATH,
    validation_split=0.2,
    subset="training",
    seed=SEED,
    image_size=IMG_SIZE,
    batch_size=BATCH_SIZE,
    class_names=["not_cenk", "cenk"],
    label_mode="binary"
)

val_ds = tf.keras.utils.image_dataset_from_directory(
    DATASET_PATH,
    validation_split=0.2,
    subset="validation",
    seed=SEED,
    image_size=IMG_SIZE,
    batch_size=BATCH_SIZE,
    class_names=["not_cenk", "cenk"],
    label_mode="binary"
)

print("Classes:", train_ds.class_names)

train_labels = np.concatenate([y.numpy().flatten() for _, y in train_ds])
counts = np.bincount(train_labels.astype(int), minlength=2)
total = counts.sum()

class_weight = {
    0: total / (2 * max(counts[0], 1)),
    1: total / (2 * max(counts[1], 1))
}

print("Class weights:", class_weight)

AUTOTUNE = tf.data.AUTOTUNE
train_ds = train_ds.prefetch(AUTOTUNE)
val_ds = val_ds.prefetch(AUTOTUNE)

model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(112, 112, 3)),

    tf.keras.layers.RandomFlip("horizontal"),
    tf.keras.layers.RandomRotation(0.05),
    tf.keras.layers.RandomZoom(0.10),

    tf.keras.layers.Rescaling(1.0 / 255.0),

    tf.keras.layers.Conv2D(16, 3, activation="relu", padding="same"),
    tf.keras.layers.MaxPooling2D(),

    tf.keras.layers.Conv2D(32, 3, activation="relu", padding="same"),
    tf.keras.layers.MaxPooling2D(),

    tf.keras.layers.Conv2D(64, 3, activation="relu", padding="same"),
    tf.keras.layers.MaxPooling2D(),

    tf.keras.layers.GlobalAveragePooling2D(),
    tf.keras.layers.Dense(32, activation="relu"),
    tf.keras.layers.Dense(1, activation="sigmoid")
])

model.compile(
    optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
    loss="binary_crossentropy",
    metrics=[
        "accuracy",
        tf.keras.metrics.Precision(name="precision"),
        tf.keras.metrics.Recall(name="recall")
    ]
)

model.summary()

callbacks = [
    tf.keras.callbacks.EarlyStopping(
        monitor="val_loss",
        patience=6,
        restore_best_weights=True
    ),
    tf.keras.callbacks.ReduceLROnPlateau(
        monitor="val_loss",
        factor=0.5,
        patience=3,
        min_lr=0.00001
    )
]

history = model.fit(
    train_ds,
    validation_data=val_ds,
    epochs=EPOCHS,
    class_weight=class_weight,
    callbacks=callbacks
)

model.save("cenk_classifier.keras")

print("Saved: cenk_classifier.keras")
