#!/usr/bin/env python3
"""依次切换 config.h 中的眼睛资源并编译，验证全部 .h 可构建。"""

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "main" / "config.h"
IDF_PY = Path(
    r"C:\Users\Administrator\esp\v5.1.4\esp-idf\tools\idf.py"
)
PYTHON = Path(
    r"C:\Users\Administrator\.espressif\python_env\idf5.1_py3.11_env\Scripts\python.exe"
)

EYES = [
    ("defaultEye", "data/defaultEye.h"),
    ("dragonEye", "data/dragonEye.h"),
    ("noScleraEye", "data/noScleraEye.h"),
    ("goatEye", "data/goatEye.h"),
    ("newtEye", "data/newtEye.h"),
    ("terminatorEye", "data/terminatorEye.h"),
    ("catEye", "data/catEye.h"),
    ("owlEye", "data/owlEye.h"),
    ("naugaEye", "data/naugaEye.h"),
    ("doeEye", "data/doeEye.h"),
]

INCLUDE_RE = re.compile(
    r'^(\s*//)?\s*#include\s+"(data/[^"]+\.h)"\s*(//.*)?$',
    re.MULTILINE,
)


def read_config() -> str:
    return CONFIG.read_text(encoding="utf-8")


def write_config(text: str) -> None:
    CONFIG.write_text(text, encoding="utf-8")


def set_active_eye(text: str, active_include: str) -> str:
    def repl(match: re.Match) -> str:
        inc = match.group(2)
        comment = match.group(3) or ""
        if inc == active_include:
            return f'#include "{inc}"{comment}'
        return f'//#include "{inc}"{comment}'

    return INCLUDE_RE.sub(repl, text)


def build_eye(name: str, include_path: str) -> tuple[bool, str]:
    original = read_config()
    try:
        write_config(set_active_eye(original, include_path))
        env = {
            **dict(**__import__("os").environ),
            "IDF_PATH": r"C:\Users\Administrator\esp\v5.1.4\esp-idf",
            "IDF_PYTHON_ENV_PATH": r"C:\Users\Administrator\.espressif\python_env\idf5.1_py3.11_env",
        }
        path_prefix = (
            r"C:\Users\Administrator\.espressif\tools\cmake\3.24.0\bin;"
            r"C:\Users\Administrator\.espressif\tools\ninja\1.10.2;"
            r"C:\Users\Administrator\.espressif\tools\xtensa-esp32s3-elf\esp-12.2.0_20230208\xtensa-esp32s3-elf\bin;"
        )
        env["PATH"] = path_prefix + env.get("PATH", "")

        proc = subprocess.run(
            [str(PYTHON), str(IDF_PY), "-C", str(ROOT), "build"],
            env=env,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
        ok = proc.returncode == 0
        tail = (proc.stdout + proc.stderr)[-2000:]
        return ok, tail
    finally:
        write_config(original)


def main() -> int:
    results: list[tuple[str, bool, str]] = []
    for name, inc in EYES:
        print(f"=== building {name} ({inc}) ===", flush=True)
        ok, tail = build_eye(name, inc)
        results.append((name, ok, tail))
        status = "OK" if ok else "FAIL"
        print(f"  -> {status}\n", flush=True)

    print("\n========== SUMMARY ==========")
    failed = 0
    for name, ok, tail in results:
        mark = "OK  " if ok else "FAIL"
        print(f"  {mark}  {name}")
        if not ok:
            failed += 1
            print(tail)

    out = ROOT / "tools" / "build_all_eyes_report.txt"
    lines = [f"{'OK' if ok else 'FAIL'}\t{name}" for name, ok, _ in results]
    out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"\nReport: {out}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
