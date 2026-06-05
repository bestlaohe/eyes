import re
import sys
import time
from pathlib import Path

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM12"
BAUD = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
DURATION = float(sys.argv[3]) if len(sys.argv) > 3 else 12.0
OUT = Path(sys.argv[4]) if len(sys.argv) > 4 else Path("monitor_out.txt")

ser = serial.Serial(PORT, BAUD, timeout=0.2)
time.sleep(0.5)

buf = ""
fps_values = []
t0 = time.time()
while time.time() - t0 < DURATION:
    data = ser.read(4096)
    if data:
        text = data.decode("utf-8", errors="replace")
        buf += text
        for m in re.finditer(r"(?:frame FPS|FPS):\s*(\d+)", text, re.IGNORECASE):
            fps_values.append(int(m.group(1)))

ser.close()
OUT.write_text(buf, encoding="utf-8")

summary = "SUMMARY lines=%d fps_samples=%d max_fps=%s last_fps=%s pass_100=%s" % (
    len(buf.splitlines()),
    len(fps_values),
    max(fps_values) if fps_values else "n/a",
    fps_values[-1] if fps_values else "n/a",
    "yes" if fps_values and max(fps_values) >= 100 else "no",
)
print(summary)
