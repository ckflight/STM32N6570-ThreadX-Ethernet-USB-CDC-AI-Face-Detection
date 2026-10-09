
import os
import cv2
import random
from pathlib import Path

FACE_SIZE           = 112
CENK_SOURCE         = Path("/home/ck/Downloads")#Path("Face_Images")
NEGATIVE_SOURCE     = Path("/home/ck/Downloads/img_align_celeba")
DATASET             = Path("/home/ck/Downloads/dataset")
MAX_NEGATIVE_IMAGES = 3000
SEED                = 42

random.seed(SEED)

cenk_dir                = DATASET / "cenk"
not_cenk_dir            = DATASET / "not_cenk"

cenk_dir.mkdir(parents=True, exist_ok=True)
not_cenk_dir.mkdir(parents=True, exist_ok=True)

detector = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")

def get_images(folder):
    return sorted([p for p in folder.rglob("*") if p.suffix.lower() in [".jpg", ".jpeg", ".png"]])

def extract_face(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    faces = detector.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

    if len(faces) == 0:
        return None

    x, y, w, h = max(faces, key=lambda f: f[2] * f[3])
    face = img[y:y+h, x:x+w]
    return cv2.resize(face, (FACE_SIZE, FACE_SIZE), interpolation=cv2.INTER_AREA)

def augment_cenk(face):
    variants = [
        face,
        cv2.flip(face, 1),
        cv2.convertScaleAbs(face, alpha=0.75, beta=-15),
        cv2.convertScaleAbs(face, alpha=1.15, beta=15),
        cv2.convertScaleAbs(face, alpha=0.80, beta=20),
        cv2.convertScaleAbs(face, alpha=1.25, beta=-20)
    ]

    hsv = cv2.cvtColor(face, cv2.COLOR_BGR2HSV)
    hsv[:, :, 1] = cv2.multiply(hsv[:, :, 1], 1.20)
    variants.append(cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR))

    return variants

cenk_count = 0

for path in get_images(CENK_SOURCE):
    img = cv2.imread(str(path))
    if img is None:
        continue

    face = extract_face(img)
    if face is None:
        print("Face not found:", path)
        continue

    for j, variant in enumerate(augment_cenk(face)):
        cv2.imwrite(str(cenk_dir / f"cenk_{cenk_count:05d}_{j}.jpg"), variant)

    cenk_count += 1

print("CENK original images:", cenk_count)
print("CENK augmented images:", cenk_count * 7)

negative_paths = get_images(NEGATIVE_SOURCE)
random.shuffle(negative_paths)

negative_count = 0

for path in negative_paths:
    if negative_count >= MAX_NEGATIVE_IMAGES:
        break

    img = cv2.imread(str(path))
    if img is None:
        continue

    # CelebA aligned images are already centered on faces.
    face = cv2.resize(img, (FACE_SIZE, FACE_SIZE), interpolation=cv2.INTER_AREA)
    cv2.imwrite(str(not_cenk_dir / f"not_cenk_{negative_count:05d}.jpg"), face)

    negative_count += 1

print("NOT_CENK images:", negative_count)
print("Dataset preparation completed.")
