# 月薪喵表情包资源（MFuns 转载合集）

来源：[MFuns 第1弹](https://www.mfuns.net/article/120254)（文章 ID `120254`，共 62 个 GIF）

**版权说明：** 表情包原作者为抖音博主「月薪喵」，基于自家猫咪创作。本站资源仅供个人学习与小设备演示，请支持原作者。

## 目录结构

```
assets/salary_cat/
  manifest.json       # 下载清单与来源 URL
  vol1/               # 第1弹 62 个 GIF 原文件
```

## 下载 / 更新

```bash
python tools/download_salary_cat_gifs.py
```

## 转为 ESP32 RGB565 帧数据

单个 GIF（并同步更新 `main/data/salaryCatFrames.h` 供固件默认播放）：

```bash
python tools/build_salary_cat_clips.py --gif 00_76dec3740034.gif --legacy-out
```

批量转换（生成 `main/data/salary_cat/*.h` 与目录索引；体积较大，按需执行）：

```bash
python tools/build_salary_cat_clips.py --volume vol1
```

> 62 个 GIF 全部转成 C 头文件约 30MB+，无法一次性烧进 ESP32 Flash。建议：仓库保留 GIF 原文件，固件只嵌入当前要播的 1～几个 clip，或通过 SPIFFS 分区加载。

## 第2/3弹

若需下载其它合集，可在 `tools/download_salary_cat_gifs.py` 的 `articles` 列表追加 `(article_id, volume)` 后重新运行。
