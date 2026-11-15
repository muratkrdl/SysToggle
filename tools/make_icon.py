import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

SIZE = 256
SCALE = 4
CANVAS = SIZE * SCALE

TOP_COLOR = (36, 41, 64)
BOTTOM_COLOR = (14, 16, 26)
ON_COLOR = (0, 214, 160)
ON_GLOW = (0, 214, 160, 110)
OFF_COLOR = (64, 70, 92)
KNOB_COLOR = (246, 248, 252)
LABEL_COLOR = (120, 128, 150)

ROWS = ((True, 0.31), (False, 0.50), (True, 0.69))


def vertical_gradient(size, top, bottom):
    image = Image.new("RGB", (size, size))
    draw = ImageDraw.Draw(image)
    for y in range(size):
        t = y / (size - 1)
        color = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3))
        draw.line([(0, y), (size, y)], fill=color)
    return image


def draw_rows(base):
    glow = Image.new("RGBA", base.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    shapes = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(shapes)

    track_w, track_h = int(CANVAS * 0.42), int(CANVAS * 0.135)
    track_x = int(CANVAS * 0.40)
    label_x, label_w = int(CANVAS * 0.13), int(CANVAS * 0.20)
    for on, center in ROWS:
        y0 = int(CANVAS * center - track_h / 2)
        box = [track_x, y0, track_x + track_w, y0 + track_h]
        if on:
            glow_draw.rounded_rectangle(box, radius=track_h // 2, fill=ON_GLOW)
        draw.rounded_rectangle(box, radius=track_h // 2, fill=ON_COLOR if on else OFF_COLOR)

        knob = int(track_h * 0.76)
        pad = (track_h - knob) // 2
        knob_x = track_x + track_w - pad - knob if on else track_x + pad
        draw.ellipse([knob_x, y0 + pad, knob_x + knob, y0 + pad + knob], fill=KNOB_COLOR)

        label_h = int(track_h * 0.26)
        label_y = y0 + (track_h - label_h) // 2
        width = label_w if on else int(label_w * 0.7)
        draw.rounded_rectangle([label_x, label_y, label_x + width, label_y + label_h],
                               radius=label_h // 2, fill=LABEL_COLOR)

    glow = glow.filter(ImageFilter.GaussianBlur(CANVAS * 0.03))
    base = Image.alpha_composite(base.convert("RGBA"), glow)
    return Image.alpha_composite(base, shapes)


def main():
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    image = draw_rows(vertical_gradient(CANVAS, TOP_COLOR, BOTTOM_COLOR)).convert("RGB")
    icon = image.resize((SIZE, SIZE), Image.LANCZOS)
    icon.save(out / "icon.jpg", quality=95)
    icon.resize((128, 128), Image.LANCZOS).save(out / "app_icon.png")
    icon.resize((512, 512), Image.LANCZOS).save(out / "icon_512.png")


if __name__ == "__main__":
    main()
