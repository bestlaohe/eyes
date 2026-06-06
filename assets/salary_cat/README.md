# 月薪喵 GIF 资源（MFuns vol1）

动图存放在 `vol1/`（62 个 GIF），清单见 `manifest.json`。

## 固件播放

在 `main/config.h` 修改 `GIF_CLIP_INDEX`（如 `GIF_VOL1_CRY`），然后：

```bash
idf.py build flash
```

构建时 `tools/gif_to_frames.py` 会自动把选中 GIF 转为 `main/data/gif_frames.*` 并编入固件。
