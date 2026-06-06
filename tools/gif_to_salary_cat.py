#!/usr/bin/env python3
"""Convert tools/salary_cat.gif to PROGMEM frame data for ESP32."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_GIF = ROOT / "tools" / "salary_cat.gif"
DATA_DIR = ROOT / "main" / "data"


def rgb_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def frame_to_rgb565(img: Image.Image) -> list[int]:
    rgba = img.convert("RGBA")
    pixels: list[int] = []
    for r, g, b, a in rgba.getdata():
        if a < 128:
            pixels.append(0x0000)
        else:
            pixels.append(rgb_to_rgb565(r, g, b))
    return pixels


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gif", type=Path, default=DEFAULT_GIF)
    parser.add_argument("--width", type=int, default=120)
    parser.add_argument("--height", type=int, default=120)
    parser.add_argument(
        "--step",
        type=int,
        default=4,
        help="keep every Nth GIF frame to reduce flash/ram usage",
    )
    args = parser.parse_args()

    if not args.gif.exists():
        print(f"Missing GIF: {args.gif}", file=sys.stderr)
        return 1

    im = Image.open(args.gif)
    n_frames = getattr(im, "n_frames", 1)
    step = max(1, args.step)
    frame_ms = im.info.get("duration") or 40
    if not frame_ms:
        frame_ms = 40
    frame_ms *= step

    frames: list[list[int]] = []
    for i in range(0, n_frames, step):
        im.seek(i)
        frame = im.convert("RGBA")
        if frame.size != (args.width, args.height):
            frame = frame.resize((args.width, args.height), Image.Resampling.LANCZOS)
        frames.append(frame_to_rgb565(frame))

    w, h = args.width, args.height
    header = DATA_DIR / "salaryCatFrames.h"
    inc = DATA_DIR / "salaryCatFrames.inc"

    header.write_text(
        "\n".join(
            [
                "#pragma once",
                "",
                "#include <stdint.h>",
                "",
                f"#define SALARY_CAT_FRAME_W     {w}",
                f"#define SALARY_CAT_FRAME_H     {h}",
                f"#define SALARY_CAT_FRAME_COUNT {len(frames)}",
                f"#define SALARY_CAT_FRAME_MS    {frame_ms}",
                f"#define SALARY_CAT_FRAME_BYTES (SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H * 2U)",
                "",
                "extern uint16_t salary_cat_frames[SALARY_CAT_FRAME_COUNT]"
                "[SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H];",
                "",
            ]
        ),
        encoding="utf-8",
    )

    lines = [
        "// 帧数据放在内部 DRAM，避免 SPI 刷屏时从 Flash 读 PROGMEM 卡死",
        "uint16_t salary_cat_frames[SALARY_CAT_FRAME_COUNT]"
        "[SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H] = {",
    ]
    for fi, pixels in enumerate(frames):
        lines.append(f"  // frame {fi}")
        for i in range(0, len(pixels), 12):
            chunk = pixels[i : i + 12]
            hexes = ", ".join(f"0x{v:04X}" for v in chunk)
            lines.append(f"  {hexes},")
    lines.append("};")
    lines.append("")
    inc.write_text("\n".join(lines), encoding="utf-8")

    total_kb = len(frames) * w * h * 2 // 1024
    print(f"Wrote {header.name}, {inc.name}: {len(frames)} frames {w}x{h}, {total_kb} KB")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
