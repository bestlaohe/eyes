#!/usr/bin/env python3
"""Build salary_cat frame data from assets/salary_cat vol1 clips (config.h macro selects clip)."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONVERTER = Path(__file__).resolve().parent / "gif_to_salary_cat.py"


def main() -> int:
    cmd = [sys.executable, str(CONVERTER)]
    if len(sys.argv) > 1:
        cmd.extend(sys.argv[1:])
    return subprocess.call(cmd, cwd=ROOT)


if __name__ == "__main__":
    raise SystemExit(main())
