#!/usr/bin/env python3
"""从 MFuns 文章页抓取月薪喵 GIF，写入 assets/salary_cat/volN 并更新 manifest.json。"""

from __future__ import annotations

import argparse
import json
import re
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASSET_ROOT = ROOT / "assets" / "salary_cat"
MANIFEST = ASSET_ROOT / "manifest.json"

UA = {"User-Agent": "Mozilla/5.0 (compatible; eyes-import/1.0)"}


def scrape_gif_hashes(article_url: str) -> list[str]:
    req = urllib.request.Request(article_url, headers=UA)
    html = urllib.request.urlopen(req, timeout=30).read().decode("utf-8", errors="replace")
    hashes = sorted(set(re.findall(r"https://cdn\.mfuns\.net/static/([a-f0-9]+)\.gif", html)))
    if not hashes:
        raise RuntimeError(f"未在页面找到 GIF: {article_url}")
    return hashes


def article_id_from_url(url: str) -> str:
    m = re.search(r"/article/(\d+)", url)
    if not m:
        raise ValueError(f"无法解析文章 ID: {url}")
    return m.group(1)


def import_volume(*, article_url: str, volume: str) -> int:
    article_id = article_id_from_url(article_url)
    vol_dir = ASSET_ROOT / volume
    vol_dir.mkdir(parents=True, exist_ok=True)

    hashes = scrape_gif_hashes(article_url)
    items: list[dict] = []
    for i, h in enumerate(hashes):
        short = h[:12]
        fname = f"{i:02d}_{short}.gif"
        rel = f"{volume}/{fname}"
        dest = ASSET_ROOT / rel
        src_url = f"https://cdn.mfuns.net/static/{h}.gif"
        if not dest.exists() or dest.stat().st_size < 100:
            print(f"download {i + 1}/{len(hashes)} {rel}")
            data = urllib.request.urlopen(src_url, timeout=60).read()
            dest.write_bytes(data)
        items.append(
            {
                "index": i,
                "file": rel,
                "source_url": src_url,
                "article_id": article_id,
                "article_url": article_url,
            }
        )

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8")) if MANIFEST.exists() else {
        "source": "https://www.mfuns.net",
        "attribution": "抖音博主「月薪喵」原创表情包；MFuns 社区转载合集",
        "license_note": "仅供个人学习/设备演示，请支持原作者",
        "volumes": [],
    }
    volumes = [v for v in manifest.get("volumes", []) if v.get("volume") != volume]
    volumes.append(
        {
            "article_id": article_id,
            "article_url": article_url,
            "volume": volume,
            "count": len(items),
            "items": items,
        }
    )
    volumes.sort(key=lambda v: v["volume"])
    manifest["volumes"] = volumes
    MANIFEST.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {MANIFEST.relative_to(ROOT)}: {volume} {len(items)} clips")
    return len(items)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--article", required=True, help="MFuns 文章 URL")
    parser.add_argument("--volume", required=True, help="卷名，如 vol2")
    args = parser.parse_args()
    try:
        import_volume(article_url=args.article, volume=args.volume)
    except Exception as exc:
        print(exc, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
