# 月薪喵 GIF 资源（MFuns）

| 卷 | 文章 | 数量 | 目录 |
|----|------|------|------|
| vol1 第一弹 | [120254](https://www.mfuns.net/article/120254) | 62 | `vol1/` |
| vol2 第二弹 | [120319](https://www.mfuns.net/article/120319) | 69 | `vol2/` |
| vol3 第三弹 | [120326](https://www.mfuns.net/article/120326) | 70 | `vol3/` |

清单见 `manifest.json`。原作者：抖音「月薪喵」。

## 固件播放

在 `main/config.h` 设置卷与序号，例如：

```c
#define GIF_CLIP_VOL     3
#define GIF_CLIP_INDEX   GIF_VOL3_00
```

然后 `idf.py build flash`。构建时 `tools/gif_to_frames.py` 会生成 `main/data/gif_catalog.h` 与当前 GIF 的帧数据。

## 导入新卷

```bash
python tools/import_mfuns_vol.py --article https://www.mfuns.net/article/120326 --volume vol3
python tools/gif_to_frames.py --gen-catalog
```
