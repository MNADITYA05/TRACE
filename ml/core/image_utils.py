import cv2


def load_and_resize(image_path: str, max_dim: int = 400):
    """Load an image from disk and resize if either dimension exceeds max_dim."""
    image = cv2.imread(image_path)
    if image is None:
        return None

    h, w, _ = image.shape
    if max(h, w) > max_dim:
        scale = max_dim / max(h, w)
        image = cv2.resize(image, (int(w * scale), int(h * scale)), interpolation=cv2.INTER_AREA)

    return image


def bgr_to_rgb(image):
    return cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
