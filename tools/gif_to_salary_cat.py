#!/usr/bin/env python3
"""Convert a GIF to internal-RAM RGB565 frame data for ESP32 salary_cat player."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_GIF = ROOT / "tools" / "salary_cat.gif"
DATA_DIR = ROOT / "main" / "data"
MANIFEST = ROOT / "assets" / "salary_cat" / "manifest.json"
CONFIG_H = ROOT / "main" / "config.h"
VOL1_HEADER = DATA_DIR / "salary_cat_vol1.h"


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


def load_manifest_vol1() -> list[dict]:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    for vol in data.get("volumes", []):
        if vol.get("volume") == "vol1":
            return vol["items"]
    raise ValueError(f"vol1 not found in {MANIFEST}")


def vol1_gif_path(index: int) -> Path:
    items = load_manifest_vol1()
    if index < 0 or index >= len(items):
        raise IndexError(f"vol1 clip index {index} out of range 0..{len(items) - 1}")
    return ROOT / "assets" / "salary_cat" / items[index]["file"]


def write_vol1_header() -> int:
    items = load_manifest_vol1()
    lines = [
        "#pragma once",
        "",
        "// 由 tools/build_salary_cat_clips.py 根据 assets/salary_cat/manifest.json 生成",
        f"#define SALARY_CAT_VOL1_COUNT {len(items)}",
        "",
    ]
    for item in items:
        idx = item["index"]
        stem = Path(item["file"]).stem.replace("-", "_")
        macro = f"SALARY_CAT_VOL1_{idx:02d}"
        lines.append(f"#define {macro} {idx}")
        lines.append(f'#define {macro}_FILE "{item["file"]}"')
    lines.append("")
    lines.append("static const char *const SALARY_CAT_VOL1_PATHS[SALARY_CAT_VOL1_COUNT] = {")
    for item in items:
        lines.append(f'  "{item["file"]}",')
    lines.append("};")
    lines.append("")
    VOL1_HEADER.write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote {VOL1_HEADER.relative_to(ROOT)} ({len(items)} clips)")
    return len(items)


def resolve_clip_index(raw: str, vol1_macros: dict[str, int]) -> int:
    raw = raw.strip()
    if raw.isdigit():
        return int(raw)
    if raw in vol1_macros:
        return vol1_macros[raw]
    raise ValueError(f"unknown clip selector: {raw}")


def read_clip_index_from_config() -> int | None:
    if not CONFIG_H.exists():
        return None
    text = CONFIG_H.read_text(encoding="utf-8")
    m = re.search(r"^#define\s+SALARY_CAT_CLIP_INDEX\s+(\S+)", text, re.MULTILINE)
    if not m:
        return None
    token = m.group(1)
    if token.isdigit():
        return int(token)
    if VOL1_HEADER.exists():
        macros = {}
        for line in VOL1_HEADER.read_text(encoding="utf-8").splitlines():
            mm = re.match(r"#define\s+(SALARY_CAT_VOL1_\d+)\s+(\d+)", line)
            if mm:
                macros[mm.group(1)] = int(mm.group(2))
        if token in macros:
            return macros[token]
    return None


def read_int_macro(name: str, default: int) -> int:
    if not CONFIG_H.exists():
        return default
    text = CONFIG_H.read_text(encoding="utf-8")
    m = re.search(rf"^#define\s+{name}\s+(\d+)", text, re.MULTILINE)
    return int(m.group(1)) if m else default


def extract_frames(im: Image.Image, width: int, height: int, step: int) -> list[list[int]]:
    n_frames = getattr(im, "n_frames", 1)
    frames: list[list[int]] = []
    for i in range(0, n_frames, step):
        im.seek(i)
        frame = im.convert("RGBA")
        if frame.size != (width, height):
            frame = frame.resize((width, height), Image.Resampling.LANCZOS)
        frames.append(frame_to_rgb565(frame))
    return frames


def convert_gif(
    gif_path: Path,
    *,
    width: int,
    height: int,
    step: int,
    max_dram_kb: int = 240,
    clip_index: int | None = None,
    clip_source: str | None = None,
) -> int:
    if not gif_path.exists():
        print(f"Missing GIF: {gif_path}", file=sys.stderr)
        return 1

    im = Image.open(gif_path)
    n_frames = getattr(im, "n_frames", 1)
    requested_step = max(1, step)
    step = requested_step
    while True:
        frames = extract_frames(im, width, height, step)
        total_kb = len(frames) * width * height * 2 // 1024
        if total_kb <= max_dram_kb or step >= n_frames:
            if step > requested_step:
                print(
                    f"Note: step {requested_step}->{step} to fit {total_kb} KB "
                    f"(limit {max_dram_kb} KB)",
                    file=sys.stderr,
                )
            break
        step += 1

    frame_ms = im.info.get("duration") or 40
    if not frame_ms:
        frame_ms = 40
    frame_ms *= step

    header = DATA_DIR / "salaryCatFrames.h"
    inc = DATA_DIR / "salaryCatFrames.inc"

    meta: list[str] = [
        "#pragma once",
        "",
        '#include "salary_cat_vol1.h"',
        "",
        "#include <stdint.h>",
        "",
        f"// source: {clip_source or gif_path.name}",
    ]
    if clip_index is not None:
        meta.append(f"#define SALARY_CAT_ACTIVE_CLIP_INDEX {clip_index}")
    fps_x10 = (10000 + frame_ms // 2) // frame_ms if frame_ms else 0
    meta.extend(
        [
            "",
            f"#define SALARY_CAT_FRAME_W     {width}",
            f"#define SALARY_CAT_FRAME_H     {height}",
            f"#define SALARY_CAT_FRAME_COUNT {len(frames)}",
            f"#define SALARY_CAT_FRAME_MS    {frame_ms}",
            f"#define SALARY_CAT_FRAME_STEP_USED {step}",
            f"#define SALARY_CAT_FRAME_FPS_x10 {fps_x10}",
            f"#define SALARY_CAT_FRAME_BYTES (SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H * 2U)",
            "",
            "extern uint16_t salary_cat_frames[SALARY_CAT_FRAME_COUNT]"
            "[SALARY_CAT_FRAME_W * SALARY_CAT_FRAME_H];",
            "",
        ]
    )
    header.write_text("\n".join(meta), encoding="utf-8")

    lines = [
        f"// {clip_source or gif_path.name}",
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

    total_kb = len(frames) * width * height * 2 // 1024
    print(
        f"Wrote {header.name}, {inc.name}: clip={clip_index} "
        f"{len(frames)} frames {width}x{height} step={step} "
        f"{frame_ms}ms {fps_x10 / 10:.1f}fps, {total_kb} KB"
    )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gif", type=Path, default=None, help="single GIF path")
    parser.add_argument("--vol1-index", type=int, default=None, help="vol1 manifest index 0..61")
    parser.add_argument("--gen-vol1-header", action="store_true", help="only regenerate salary_cat_vol1.h")
    parser.add_argument("--width", type=int, default=None)
    parser.add_argument("--height", type=int, default=None)
    parser.add_argument("--step", type=int, default=None, help="keep every Nth GIF frame")
    args = parser.parse_args()

    if args.gen_vol1_header:
        write_vol1_header()
        return 0

    width = args.width if args.width is not None else read_int_macro("SALARY_CAT_DRAW_W", 100)
    height = args.height if args.height is not None else read_int_macro("SALARY_CAT_DRAW_H", 100)
    step = args.step if args.step is not None else read_int_macro("SALARY_CAT_FRAME_STEP", 1)
    max_dram_kb = read_int_macro("SALARY_CAT_MAX_DRAM_KB", 240)

    write_vol1_header()

    clip_index: int | None = args.vol1_index
    gif_path = args.gif
    clip_source: str | None = None

    if clip_index is None:
        clip_index = read_clip_index_from_config()

    if clip_index is not None:
        items = load_manifest_vol1()
        gif_path = vol1_gif_path(clip_index)
        clip_source = items[clip_index]["file"]
    elif gif_path is None:
        gif_path = DEFAULT_GIF

    return convert_gif(
        gif_path,
        width=width,
        height=height,
        step=step,
        max_dram_kb=max_dram_kb,
        clip_index=clip_index,
        clip_source=clip_source,
    )


if __name__ == "__main__":
    raise SystemExit(main())
