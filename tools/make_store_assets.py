import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

TOP = (36, 41, 64)
BOTTOM = (12, 14, 22)
ACCENT = (0, 214, 160)
TEXT = (240, 242, 248)
SUBTEXT = (150, 158, 184)
SCALE = 2


def gradient(width, height):
    image = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(image)
    for y in range(height):
        t = y / max(1, height - 1)
        draw.line([(0, y), (width, y)], fill=tuple(int(TOP[i] + (BOTTOM[i] - TOP[i]) * t) for i in range(3)))
    return image


def font(name, size):
    return ImageFont.truetype(str(Path("C:/Windows/Fonts") / name), size)


def compose(icon, width, height, icon_size, title_size, tagline, tagline_size):
    w, h = width * SCALE, height * SCALE
    canvas = gradient(w, h)
    draw = ImageDraw.Draw(canvas)
    size = icon_size * SCALE
    pad = (h - size) // 2
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, size, size], radius=size // 5, fill=255)
    canvas.paste(icon.resize((size, size), Image.LANCZOS), (pad, pad), mask)
    text_x = pad * 2 + size
    title_font = font("segoeuib.ttf", title_size * SCALE)
    if tagline:
        draw.text((text_x, h // 2), "SysToggle", font=title_font, fill=TEXT, anchor="ls")
        draw.text((text_x, h // 2 + 14 * SCALE), tagline, font=font("segoeui.ttf", tagline_size * SCALE),
                  fill=SUBTEXT, anchor="lt")
        draw.rectangle([0, h - 6 * SCALE, w, h], fill=ACCENT)
    else:
        draw.text((text_x, h // 2), "SysToggle", font=title_font, fill=TEXT, anchor="lm")
    return canvas.resize((width, height), Image.LANCZOS)


def main():
    icon = Image.open(sys.argv[1]).convert("RGB")
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    compose(icon, 848, 208, 160, 58, "Turn sysmodules on and off from the console", 26).save(out / "banner_848x208.png")
    centered_icon(icon, 256, 150, 124).save(out / "icon_256x150.png")


def centered_icon(icon, width, height, icon_size):
    w, h, size = width * SCALE, height * SCALE, icon_size * SCALE
    canvas = gradient(w, h)
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, size, size], radius=size // 5, fill=255)
    canvas.paste(icon.resize((size, size), Image.LANCZOS), ((w - size) // 2, (h - size) // 2), mask)
    return canvas.resize((width, height), Image.LANCZOS)


if __name__ == "__main__":
    main()
