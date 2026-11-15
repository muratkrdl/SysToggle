import math
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

SIZE = 256
SCALE = 4
CANVAS = SIZE * SCALE


def gradient(top, bottom, diagonal=False):
    image = Image.new("RGB", (CANVAS, CANVAS))
    pixels = image.load()
    for y in range(CANVAS):
        for x in range(0, CANVAS, 4):
            t = ((x + y) / (2 * CANVAS)) if diagonal else y / CANVAS
            color = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3))
            for dx in range(4):
                pixels[x + dx, y] = color
    return image.convert("RGBA")


def glow_layer(draw_fn, radius):
    layer = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
    draw_fn(ImageDraw.Draw(layer))
    return layer.filter(ImageFilter.GaussianBlur(radius))


def toggle(draw, x, y, w, h, on, on_color, off_color, knob_color=(246, 248, 252)):
    draw.rounded_rectangle([x, y, x + w, y + h], radius=h // 2, fill=on_color if on else off_color)
    knob = int(h * 0.76)
    pad = (h - knob) // 2
    kx = x + w - pad - knob if on else x + pad
    draw.ellipse([kx, y + pad, kx + knob, y + pad + knob], fill=knob_color)


def concept_big_toggle():
    base = gradient((20, 184, 140), (14, 92, 120), diagonal=True)
    w, h = int(CANVAS * 0.70), int(CANVAS * 0.34)
    x, y = (CANVAS - w) // 2, (CANVAS - h) // 2
    shadow = glow_layer(lambda d: d.rounded_rectangle([x, y + 30, x + w, y + h + 30], radius=h // 2,
                                                     fill=(0, 0, 0, 120)), 40)
    base = Image.alpha_composite(base, shadow)
    draw = ImageDraw.Draw(base)
    draw.rounded_rectangle([x, y, x + w, y + h], radius=h // 2, fill=(8, 60, 66))
    knob = int(h * 0.80)
    pad = (h - knob) // 2
    kx = x + w - pad - knob
    draw.ellipse([kx, y + pad, kx + knob, y + pad + knob], fill=(255, 255, 255))
    cx, cy, r = kx + knob // 2, y + h // 2, int(knob * 0.22)
    draw.arc([cx - r, cy - r, cx + r, cy + r], start=-60, end=240, fill=(20, 150, 125), width=int(r * 0.35))
    draw.line([cx, cy - r - 8, cx, cy - 6], fill=(20, 150, 125), width=int(r * 0.35))
    return base


def concept_module_cards():
    base = gradient((32, 30, 56), (12, 12, 22))
    draw = ImageDraw.Draw(base)
    colors = [(0, 214, 160), (255, 92, 92), (0, 214, 160)]
    states = [True, False, True]
    for i, (color, on) in enumerate(zip(colors, states)):
        y = int(CANVAS * (0.20 + i * 0.22))
        x0, x1 = int(CANVAS * 0.14), int(CANVAS * 0.86)
        h = int(CANVAS * 0.17)
        draw.rounded_rectangle([x0, y, x1, y + h], radius=int(h * 0.28), fill=(44, 46, 72))
        dot = int(h * 0.36)
        draw.ellipse([x0 + int(h * 0.32), y + (h - dot) // 2, x0 + int(h * 0.32) + dot, y + (h + dot) // 2],
                     fill=color)
        draw.rounded_rectangle([x0 + int(h * 0.95), y + int(h * 0.40), x0 + int(h * 2.0), y + int(h * 0.60)],
                               radius=int(h * 0.1), fill=(120, 126, 160))
        toggle(draw, x1 - int(h * 1.75), y + int(h * 0.22), int(h * 1.45), int(h * 0.56), on,
               (0, 214, 160), (90, 94, 120))
    return base


def concept_chip_power():
    base = gradient((24, 28, 44), (8, 10, 18))
    glow = glow_layer(lambda d: d.rounded_rectangle(
        [int(CANVAS * 0.27), int(CANVAS * 0.27), int(CANVAS * 0.73), int(CANVAS * 0.73)],
        radius=int(CANVAS * 0.07), fill=(0, 214, 160, 140)), 60)
    base = Image.alpha_composite(base, glow)
    draw = ImageDraw.Draw(base)
    c0, c1 = int(CANVAS * 0.27), int(CANVAS * 0.73)
    pin_w, pin_l = int(CANVAS * 0.035), int(CANVAS * 0.08)
    for i in range(4):
        p = c0 + int((c1 - c0) * (0.18 + i * 0.213))
        for box in ([p, c0 - pin_l, p + pin_w, c0], [p, c1, p + pin_w, c1 + pin_l],
                    [c0 - pin_l, p, c0, p + pin_w], [c1, p, c1 + pin_l, p + pin_w]):
            draw.rounded_rectangle(box, radius=pin_w // 3, fill=(130, 140, 170))
    draw.rounded_rectangle([c0, c0, c1, c1], radius=int(CANVAS * 0.07), fill=(36, 40, 62),
                           outline=(0, 214, 160), width=int(CANVAS * 0.012))
    cx = cy = CANVAS // 2
    r = int(CANVAS * 0.12)
    width = int(CANVAS * 0.028)
    draw.arc([cx - r, cy - r, cx + r, cy + r], start=-55, end=235, fill=(0, 214, 160), width=width)
    draw.rounded_rectangle([cx - width // 2, cy - r - width, cx + width // 2, cy - int(r * 0.15)],
                           radius=width // 2, fill=(0, 214, 160))
    return base


def concept_split_dual():
    base = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 255))
    draw = ImageDraw.Draw(base)
    draw.rectangle([0, 0, CANVAS // 2, CANVAS], fill=(0, 190, 150))
    draw.rectangle([CANVAS // 2, 0, CANVAS, CANVAS], fill=(40, 44, 66))
    w, h = int(CANVAS * 0.30), int(CANVAS * 0.15)
    toggle(draw, CANVAS // 4 - w // 2, CANVAS // 2 - h // 2, w, h, True, (0, 120, 96), (0, 0, 0),
           knob_color=(255, 255, 255))
    toggle(draw, 3 * CANVAS // 4 - w // 2, CANVAS // 2 - h // 2, w, h, False, (0, 0, 0), (22, 24, 38),
           knob_color=(150, 156, 184))
    return base


def concept_three_toggles():
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import make_icon
    return make_icon.draw_rows(make_icon.vertical_gradient(CANVAS, make_icon.TOP_COLOR, make_icon.BOTTOM_COLOR))


EXPORT_SIZES = (1024, 512, 256, 128)

CONCEPTS = [
    ("A", "Uc anahtar", concept_three_toggles),
    ("B", "Tek buyuk anahtar", concept_big_toggle),
    ("C", "Modul kartlari", concept_module_cards),
    ("D", "Cip + guc", concept_chip_power),
    ("E", "Acik / kapali", concept_split_dual),
]


def export(key, full, out):
    folder = out / key
    folder.mkdir(parents=True, exist_ok=True)
    for size in EXPORT_SIZES:
        full.resize((size, size), Image.LANCZOS).save(folder / f"icon_{size}.png")
    full.resize((SIZE, SIZE), Image.LANCZOS).save(folder / "icon.jpg", quality=95)


def install(out, key, project_root):
    folder = out / key
    targets = {
        folder / "icon.jpg": project_root / "resources" / "icon" / "icon.jpg",
        folder / "icon_128.png": project_root / "resources" / "img" / "app_icon.png",
        folder / "icon_512.png": project_root / "docs" / "icon.png",
    }
    for source, target in targets.items():
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(source.read_bytes())
        print(f"installed {source.name} -> {target.relative_to(project_root)}")


def main():
    out = Path(sys.argv[1])
    chosen = sys.argv[3] if len(sys.argv) > 3 and sys.argv[2] == "--install" else None
    out.mkdir(parents=True, exist_ok=True)
    for stale in out.glob("concept_*.png"):
        stale.unlink()
    tiles = []
    for key, label, build in CONCEPTS:
        full = build().convert("RGB")
        export(key, full, out)
        tiles.append((key, label, full.resize((SIZE, SIZE), Image.LANCZOS)))

    pad, caption = 24, 44
    sheet = Image.new("RGB", (len(tiles) * (SIZE + pad) + pad, SIZE + 2 * pad + caption), (245, 246, 250))
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.truetype("arial.ttf", 22)
    for i, (key, label, icon) in enumerate(tiles):
        x = pad + i * (SIZE + pad)
        mask = Image.new("L", (SIZE, SIZE), 0)
        ImageDraw.Draw(mask).rounded_rectangle([0, 0, SIZE, SIZE], radius=40, fill=255)
        sheet.paste(icon, (x, pad), mask)
        draw.text((x + SIZE // 2, pad + SIZE + 14), f"{key}  {label}", fill=(30, 32, 40), font=font, anchor="ma")
    sheet.save(out / "concepts.png")
    if chosen:
        install(out, chosen, Path(__file__).resolve().parent.parent)


if __name__ == "__main__":
    main()
