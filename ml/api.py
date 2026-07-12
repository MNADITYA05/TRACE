import os
import glob
from dotenv import load_dotenv
from fastapi import FastAPI
from pydantic import BaseModel
from PIL import Image

from core.image_utils import load_and_resize, bgr_to_rgb
from core.barcode import extract_barcode, mask_barcode
from core.detector import detect_defects
from db.mongo import update_barcode

load_dotenv()

app = FastAPI()

IMAGE_DIR = os.getenv("IMAGE_DIR", "pcb_with_barcodes")


class TriggerInput(BaseModel):
    product_id: str


@app.post("/trigger")
def trigger_detection(input: TriggerInput):
    search_pattern = os.path.join(IMAGE_DIR, f"*_{input.product_id}.png")
    matches = glob.glob(search_pattern)

    if not matches:
        return {"success": False, "reason": "Image not found for product_id"}

    image_path = matches[0]

    image_bgr = load_and_resize(image_path)
    if image_bgr is None:
        return {"success": False, "reason": "Image read failed"}

    image_rgb = bgr_to_rgb(image_bgr)
    image_pil = Image.fromarray(image_rgb)

    barcode = extract_barcode(image_pil)
    if not barcode:
        return {"success": False, "reason": "OCR failed (barcode not found)"}

    masked = mask_barcode(image_rgb.copy())
    defects = detect_defects(masked)
    updated = update_barcode(barcode, defects)

    return {
        "success": True,
        "barcode": barcode,
        "quality_status": "defective" if defects else "no_defect",
        "defect_type": [d["type"] for d in defects] or ["none"],
        "updated": updated
    }
