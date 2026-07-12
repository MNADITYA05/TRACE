import os
import urllib.request
import numpy as np
from ultralytics import YOLO

MODEL_PATH = os.getenv("MODEL_PATH", os.path.join(os.path.dirname(__file__), "../models/best.pt"))
WEIGHTS_URL = "https://github.com/MNADITYA05/Smart-Automated-System-for-Product-Labeling-and-Traceability/releases/download/v1.0.0/best.pt"

_model = None


def get_model() -> YOLO:
    """Lazy-load the YOLO model. Downloads weights from GitHub Releases if not present locally."""
    if not os.path.exists(MODEL_PATH):
        os.makedirs(os.path.dirname(MODEL_PATH), exist_ok=True)
        print(f"Downloading model weights from {WEIGHTS_URL} ...")
        urllib.request.urlretrieve(WEIGHTS_URL, MODEL_PATH)
        print("Download complete.")
    global _model
    if _model is None:
        _model = YOLO(MODEL_PATH)
    return _model


def detect_defects(image_rgb: np.ndarray) -> list[dict]:
    """Run YOLO inference on an RGB image. Returns a list of {type, confidence} dicts."""
    model = get_model()
    results = model.predict(source=image_rgb, conf=0.25, verbose=False)[0]
    defects = []
    if results.boxes:
        for box in results.boxes:
            class_id = int(box.cls[0])
            confidence = float(box.conf[0])
            class_name = model.names[class_id]
            defects.append({"type": class_name, "confidence": confidence})
    return defects
