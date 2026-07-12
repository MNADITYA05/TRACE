import numpy as np
import cv2
from pyzbar.pyzbar import decode
from PIL import Image


def extract_barcode(image_pil: Image.Image) -> str | None:
    """Extract a 13-digit EAN barcode from a PIL image. Returns the barcode string or None."""
    for obj in decode(image_pil):
        text = obj.data.decode("utf-8").strip()
        if text.isdigit() and len(text) == 13:
            return text
    return None


def mask_barcode(image_rgb: np.ndarray) -> np.ndarray:
    """White-out the barcode region in an RGB image so YOLO ignores it."""
    for obj in decode(Image.fromarray(image_rgb)):
        points = obj.polygon
        if points:
            pts = np.array([[p.x, p.y] for p in points], np.int32)
            cv2.fillPoly(image_rgb, [pts], (255, 255, 255))
    return image_rgb
