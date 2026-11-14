#!/usr/bin/env python3
"""
Generates the slot masks used by shaders/mask_recolor.fs to recolour the HUD portraits.

The HUD portraits are painted with hundreds of colours, so an exact palette swap (like the one used
for the player sprites) can't find them. Instead every pixel that belongs to a customizable part is
marked in a mask image, using the slot index of that part in CharacterPalette:

    red = slot 0, green = slot 1, blue = slot 2, yellow = slot 3, transparent = not recoloured

The masks are classified by hue/saturation plus a few hand-picked regions. They are plain PNGs, so
they can also be touched up by hand in Aseprite - just keep to the four pure mask colours.

Usage: python3 tools/generate_portrait_masks.py [portrait directory]
"""
import colorsys
import sys
from pathlib import Path

from PIL import Image

SLOT_COLORS = [(255, 0, 0, 255), (0, 255, 0, 255), (0, 0, 255, 255), (255, 255, 0, 255)]

# Slot indices must match ROGUE_SLOTS / WIZARD_SLOTS in CharacterPalette.cpp.
ROGUE_CLOTH, ROGUE_SCARF, ROGUE_FACE, ROGUE_EYES = 0, 1, 2, 3
WIZARD_CLOTHES, WIZARD_BEARD = 0, 1


def classify_rogue(x, y, hue, sat, val):
    if 35 <= hue <= 80 and sat >= 0.2:
        return ROGUE_CLOTH  # Olive hood. The skin and bow are orange (~25 deg), well below this.
    if 18 <= y <= 28 and 16 <= x <= 46 and 185 <= hue <= 210 and sat >= 0.5:
        return ROGUE_EYES  # Region keeps the light blue backdrop (218 deg) out.
    if y >= 26 and 12 <= x <= 48 and (hue >= 320 or hue <= 12) and 0.2 <= sat <= 0.6:
        return ROGUE_SCARF
    return None


def classify_wizard(x, y, hue, sat, val):
    if 250 <= hue <= 335 and sat >= 0.35 and val >= 0.08:
        return WIZARD_CLOTHES  # Purple hat and robe; the hat band is orange.
    is_eye = 21 <= y <= 25 and 22 <= x <= 42  # The eye whites are grey too.
    if not is_eye and y >= 15 and sat <= 0.26 and val >= 0.15:
        return WIZARD_BEARD  # Greys; the skin is far more saturated (> 0.5).
    return None


PORTRAITS = {
    'rogue_hud': classify_rogue,
    'wizard_hud': classify_wizard,
}


def generate(directory: Path, name: str, classify) -> None:
    image = Image.open(directory / f'{name}.png').convert('RGBA')
    mask = Image.new('RGBA', image.size, (0, 0, 0, 0))
    for y in range(image.height):
        for x in range(image.width):
            r, g, b, a = image.getpixel((x, y))
            if a == 0:
                continue
            hue, sat, val = colorsys.rgb_to_hsv(r / 255, g / 255, b / 255)
            slot = classify(x, y, hue * 360, sat, val)
            if slot is not None:
                mask.putpixel((x, y), SLOT_COLORS[slot])
    mask.save(directory / f'{name}_mask.png')
    print(f'Wrote {directory / f"{name}_mask.png"}')


def main() -> None:
    default_dir = Path(__file__).resolve().parent.parent / 'content/game/content/graphics/portraits'
    directory = Path(sys.argv[1]) if len(sys.argv) > 1 else default_dir
    for name, classify in PORTRAITS.items():
        generate(directory, name, classify)


if __name__ == '__main__':
    main()
