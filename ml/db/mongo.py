import os
from datetime import datetime
from pymongo import MongoClient

MONGO_URI = os.getenv("MONGO_URI", "mongodb://localhost:27017")
_client = None


def get_collection():
    """Return the barcode collection (connection is reused across calls)."""
    global _client
    if _client is None:
        _client = MongoClient(MONGO_URI)
    return _client["BarcodeDB"]["barcode"]


def update_barcode(barcode_val: str, defects: list[dict]) -> bool:
    """Write quality_status and defect_type back to the barcode document."""
    collection = get_collection()
    quality = "defective" if defects else "no_defect"
    defect_str = ", ".join([d["type"] for d in defects]) or "none"

    result = collection.update_one(
        {"barcode": str(barcode_val)},
        {"$set": {
            "quality_status": quality,
            "defect_type": defect_str,
            "last_updated": datetime.utcnow().isoformat()
        }}
    )
    return result.modified_count > 0
