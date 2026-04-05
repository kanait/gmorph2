#!/usr/bin/env python3
"""Extract XPM blobs from src/icon.h and write PNGs for the Qt toolbar.

Outputs for each logical icon:
  - *_32.png  — original pixel size (typically 32×32)
  - *_96.png  — 3× nearest-neighbour upscale for HiDPI / sharp toolbar scaling

Requires Pillow (e.g. pip install pillow).
"""
from __future__ import annotations

import io
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Install Pillow: pip install pillow", file=sys.stderr)
    sys.exit(1)

ROOT = Path(__file__).resolve().parents[1]
ICON_H = ROOT / "src" / "icon.h"
OUT_DIR = ROOT / "src" / "qt" / "icons"

# Motif edit toolbar icons → base filename (no extension)
EDIT_ICONS: list[tuple[str, str]] = [
    ("icon_create_ppdloop", "edit_create_ppdloop"),
    ("icon_delete_ppdloop", "edit_delete_ppdloop"),
    ("icon_create_ppdface", "edit_create_ppdface"),
    ("icon_delete_ppdface", "edit_delete_ppdface"),
    ("icon_create_cvp", "edit_create_cvp"),
    ("icon_move_cvp", "edit_move_cvp"),
    ("icon_delete_cvp", "edit_delete_cvp"),
    ("icon_create_pcm_face", "edit_create_pcm_face"),
    ("icon_delete_pcm_face", "edit_delete_pcm_face"),
    ("icon_make_shortestpath", "edit_make_shortestpath"),
    ("icon_delete_shortestpath", "edit_delete_shortestpath"),
    ("icon_manu_shortestpath", "edit_manu_shortestpath"),
]


def extract_xpm_strings(text: str, array_name: str) -> list[str]:
    """Parse C string literals inside `static char * name[] = { ... };`.

    Regex on raw text fails when pixel rows contain `;` (e.g. `-%%;:>=`).
    """
    key = f"static char * {array_name}[] = {{"
    start = text.find(key)
    if start < 0:
        raise SystemExit(f"Array {array_name} not found in {ICON_H}")
    i = start + len(key)
    out: list[str] = []
    n = len(text)
    while i < n:
        c = text[i]
        if c in " \t\n\r":
            i += 1
            continue
        if c == '"':
            j = i + 1
            chunk: list[str] = []
            while j < n and text[j] != '"':
                if text[j] == "\\" and j + 1 < n:
                    chunk.append(text[j : j + 2])
                    j += 2
                    continue
                chunk.append(text[j])
                j += 1
            out.append("".join(chunk))
            i = j + 1
            continue
        if text.startswith("};", i):
            break
        if c == ",":
            i += 1
            continue
        i += 1
    if not out:
        raise SystemExit(f"No strings parsed for {array_name}")
    return out


def xpm_to_image(lines: list[str]) -> Image.Image:
    wrapped = (
        "/* XPM */\nstatic char *a[] = {\n"
        + ",\n".join('"' + line.replace("\\", "\\\\").replace('"', '\\"') + '"' for line in lines)
        + "\n};\n"
    )
    return Image.open(io.BytesIO(wrapped.encode("latin-1")))


def main() -> None:
    text = ICON_H.read_text(encoding="utf-8", errors="replace")
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    scale = 3

    for xpm_name, base in EDIT_ICONS:
        lines = extract_xpm_strings(text, xpm_name)
        im = xpm_to_image(lines).convert("RGBA")
        w, h = im.size
        im.save(OUT_DIR / f"{base}_32.png", "PNG")

        big = im.resize((w * scale, h * scale), resample=Image.Resampling.NEAREST)
        big.save(OUT_DIR / f"{base}_96.png", "PNG")
        print(f"Wrote {base}_32.png ({w}×{h}) and {base}_96.png ({w*scale}×{h*scale})")


if __name__ == "__main__":
    main()
