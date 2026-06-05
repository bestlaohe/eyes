//
// 由 Bodmer 改编的 TFT_eSPI 示例，可在任何兼容 TFT_eSPI 的处理器上运行。
// 请忽略原 Adafruit 版本中针对特定开发板的限制说明。
//
// 大部分配置（图形、引脚、显示类型等）见 config.h。
// 除非做深度定制，一般无需修改本文件。
//
// 原作者：Phil Burgess / Paint Your Dragon（Adafruit Industries），MIT 许可证。
// SPI FIFO 思路来自 Paul Stoffregen 的 ILI9341_t3 库。
// 灵感来自 David Boccabella (Marcwolf) 的舵机/OLED 混合眼球概念。
//--------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <esp_flash.h>
#include <spi_flash_mmap.h>
#include <esp_partition.h>
#include "eyes_common.h"

static uint8_t  ramLidStore[2 * SCREEN_WIDTH * SCREEN_HEIGHT];
static uint16_t ramPolarStore[IRIS_WIDTH * IRIS_HEIGHT];
static uint16_t ramIrisStore[IRIS_MAP_WIDTH * IRIS_MAP_HEIGHT];

static const esp_partition_t *scleraPart = nullptr;
static size_t                 scleraPartOff = 0;
static uint16_t               scleraRow[SCLERA_WIDTH];

#define FLASH_STAGE 4096
static uint8_t flashStage[FLASH_STAGE];

static void flashLoadChunked(void *dst, const void *src, size_t bytes, const char *tag)
{
  size_t phys = spi_flash_cache2phys(src);
  if (phys == (size_t)SPI_FLASH_CACHE2PHYS_FAIL) {
    Serial.printf("textures: %s cache2phys fail\n", tag);
    return;
  }

  const esp_partition_t *part =
    esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
  if (!part) {
    Serial.printf("textures: %s no factory part\n", tag);
    return;
  }
  size_t part_off = phys - part->address;

  for (size_t off = 0; off < bytes; off += FLASH_STAGE) {
    size_t n = bytes - off;
    if (n > FLASH_STAGE) n = FLASH_STAGE;
    esp_err_t err = esp_partition_read(part, part_off + off, flashStage, n);
    if (err != ESP_OK) {
      Serial.printf("textures: %s read err %d at %u\n", tag, (int)err, (unsigned)off);
      return;
    }
    memcpy((uint8_t *)dst + off, flashStage, n);
    yield();
  }
}

static bool texReady = false;

void loadEyeTextures(void)
{
  if (texReady) return;

  const size_t nPolar = (size_t)IRIS_WIDTH * IRIS_HEIGHT;
  const size_t nLid   = (size_t)SCREEN_WIDTH * SCREEN_HEIGHT;
  const size_t nIris  = (size_t)IRIS_MAP_WIDTH * IRIS_MAP_HEIGHT;

  scleraPart = esp_partition_find_first(ESP_PARTITION_TYPE_APP,
                                        ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
  size_t phys = spi_flash_cache2phys(sclera);
  if (phys == (size_t)SPI_FLASH_CACHE2PHYS_FAIL || !scleraPart) {
    Serial.println("textures: sclera flash map fail");
    return;
  }
  scleraPartOff = phys - scleraPart->address;

  Serial.println("textures: polar...");
  flashLoadChunked(ramPolarStore, polar, nPolar * sizeof(uint16_t), "polar");
  Serial.println("textures: polar ok");

  Serial.println("textures: iris...");
  flashLoadChunked(ramIrisStore, iris, nIris * sizeof(uint16_t), "iris");
  Serial.println("textures: iris ok");

  Serial.println("textures: lids...");
  flashLoadChunked(ramLidStore, lower, nLid, "lower");
  flashLoadChunked(ramLidStore + nLid, upper, nLid, "upper");
  Serial.println("textures: lids ok");

  texReady = true;
  Serial.println("textures: ready");
}

// 眼球移动缓入缓出曲线：3*t^2 - 2*t^3
const uint8_t ease[] = {
  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  3,
  3,  3,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  9,  9, 10, 10,
  11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 44, 45, 46, 47, 48, 50, 51, 52, 53, 54, 56, 57, 58,
  60, 61, 62, 63, 65, 66, 67, 69, 70, 72, 73, 74, 76, 77, 78, 80,
  81, 83, 84, 85, 87, 88, 90, 91, 93, 94, 96, 97, 98, 100, 101, 103,
  104, 106, 107, 109, 110, 112, 113, 115, 116, 118, 119, 121, 122, 124, 125, 127,
  128, 130, 131, 133, 134, 136, 137, 139, 140, 142, 143, 145, 146, 148, 149, 151,
  152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 167, 168, 170, 171, 172, 174,
  175, 177, 178, 179, 181, 182, 183, 185, 186, 188, 189, 190, 192, 193, 194, 195,
  197, 198, 199, 201, 202, 203, 204, 205, 207, 208, 209, 210, 211, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 224, 225, 226, 227, 228, 228, 229, 230, 231,
  232, 233, 234, 235, 236, 237, 237, 238, 239, 240, 240, 241, 242, 243, 243, 244,
  245, 245, 246, 246, 247, 248, 248, 249, 249, 250, 250, 251, 251, 251, 252, 252,
  252, 253, 253, 253, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255
};

// 初始化眼睛 ---------------------------------------------------------
void initEyes(void)
{
  Serial.println("Initialise eye objects");

  // 根据 config.h 中的 eyeInfo 列表初始化每只眼睛
  for (uint8_t e = 0; e < NUM_EYES; e++) {
    Serial.print("Create display #"); Serial.println(e);

    eye[e].tft_cs      = eyeInfo[e].select;
    eye[e].blink.state = NOBLINK;
    eye[e].xposition   = eyeInfo[e].xposition;
    eye[e].yposition   = eyeInfo[e].yposition;

    pinMode(eye[e].tft_cs, OUTPUT);
    digitalWrite(eye[e].tft_cs, HIGH); // CS 默认高

    // 若定义了单眼 wink 引脚，则一并初始化
    if (eyeInfo[e].wink >= 0) pinMode(eyeInfo[e].wink, INPUT_PULLUP);
  }

#if defined(BLINK_PIN) && (BLINK_PIN >= 0)
  pinMode(BLINK_PIN, INPUT_PULLUP); // 双眼共用的手动眨眼按钮
#endif
}

// 渲染单眼 --------------------------------------------------------------
void drawEye(
  uint8_t  e,        // 眼睛索引；0=左/1=右
  uint32_t iScale,   // 虹膜缩放系数
  uint32_t  scleraX, // 巩膜图像起始 X 偏移
  uint32_t  scleraY, // 巩膜图像起始 Y 偏移
  uint32_t  uT,      // 上眼皮遮罩阈值
  uint32_t  lT) {    // 下眼皮遮罩阈值

  uint16_t rowBuf[TFT_WIDTH > SCREEN_WIDTH ? TFT_WIDTH : SCREEN_WIDTH];
  uint32_t screenX, scleraXsave, scleraYbase;
  int32_t  irisX, irisY;
  int16_t  lidX;
  uint32_t p, a, d;
  uint32_t texX, texY, curScleraX, curScleraY;

  if (!texReady) return;

  scleraXsave = scleraX;
  scleraYbase = scleraY;

  digitalWrite(eye[e].tft_cs, LOW);
  tft.startWrite();
  for (uint32_t screenY = 0; screenY < TFT_HEIGHT; screenY++) {
    texY         = (screenY * SCREEN_HEIGHT) / TFT_HEIGHT;
    curScleraY   = scleraYbase + texY;
    irisY        = (int32_t)curScleraY - (SCLERA_HEIGHT - IRIS_HEIGHT) / 2;
    if (curScleraY < SCLERA_HEIGHT && scleraPart) {
      size_t rowOff = scleraPartOff + curScleraY * SCLERA_WIDTH * sizeof(uint16_t);
      esp_partition_read(scleraPart, rowOff, scleraRow, SCLERA_WIDTH * sizeof(uint16_t));
    }
    for (screenX = 0; screenX < TFT_WIDTH; screenX++) {
      texX       = (screenX * SCREEN_WIDTH) / TFT_WIDTH;
      curScleraX = scleraXsave + texX;
      irisX      = (int32_t)curScleraX - (SCLERA_WIDTH - IRIS_WIDTH) / 2;
      lidX       = e ? (int16_t)texX : (int16_t)(SCREEN_WIDTH - 1 - texX);
      if ((ramLidStore[texY * SCREEN_WIDTH + lidX] <= lT) ||
          (ramLidStore[SCREEN_WIDTH * SCREEN_HEIGHT + texY * SCREEN_WIDTH + lidX] <= uT)) {
        p = 0;
      } else if ((irisY < 0) || (irisY >= IRIS_HEIGHT) ||
                 (irisX < 0) || (irisX >= IRIS_WIDTH)) {
        if (curScleraY < SCLERA_HEIGHT && curScleraX < SCLERA_WIDTH) {
          p = scleraRow[curScleraX];
        } else {
          p = 0;
        }
      } else {
        p = ramPolarStore[irisY * IRIS_WIDTH + irisX];
        d = (iScale * (p & 0x7F)) / 128;
        if (d < IRIS_MAP_HEIGHT) {
          a = (IRIS_MAP_WIDTH * (p >> 7)) / 512;
          p = ramIrisStore[d * IRIS_MAP_WIDTH + a];
        } else if (curScleraY < SCLERA_HEIGHT && curScleraX < SCLERA_WIDTH) {
          p = scleraRow[curScleraX];
        } else {
          p = 0;
        }
      }
      rowBuf[screenX] = (uint16_t)(p >> 8 | p << 8);
    }
    tft.setAddrWindow(eye[e].xposition, eye[e].yposition + screenY, TFT_WIDTH, 1);
#ifdef USE_DMA
    tft.pushPixelsDMA(rowBuf, TFT_WIDTH);
#else
    tft.pushPixels(rowBuf, TFT_WIDTH);
#endif
  }
  tft.endWrite();
  digitalWrite(eye[e].tft_cs, HIGH);
}

// 更新眼睛 --------------------------------------------------------------
void updateEye (void)
{
#if defined(LIGHT_PIN) && (LIGHT_PIN >= 0) // 光敏/电位器控制瞳孔

  int16_t v = analogRead(LIGHT_PIN);       // 传感器原始读数
#ifdef LIGHT_PIN_FLIP
  v = 1023 - v;                            // 反转传感器方向
#endif
  if (v < LIGHT_MIN)      v = LIGHT_MIN; // 限制传感器读数范围
  else if (v > LIGHT_MAX) v = LIGHT_MAX;
  v -= LIGHT_MIN;  // 归一化到 0 ~ (LIGHT_MAX - LIGHT_MIN)
#ifdef LIGHT_CURVE  // 对传感器输入做伽马曲线
  v = (int16_t)(pow((double)v / (double)(LIGHT_MAX - LIGHT_MIN),
                    LIGHT_CURVE) * (double)(LIGHT_MAX - LIGHT_MIN));
#endif
  // 映射到瞳孔尺寸范围（IRIS_MAX 对应最亮时的大小）
  v = map(v, 0, (LIGHT_MAX - LIGHT_MIN), IRIS_MAX, IRIS_MIN);
#ifdef IRIS_SMOOTH // 平滑滤波，瞳孔变化更平缓
  static int16_t irisValue = (IRIS_MIN + IRIS_MAX) / 2;
  irisValue = ((irisValue * 15) + v) / 16;
  frame(irisValue);
#else // 无滤波，立即跟随
  frame(v);
#endif // IRIS_SMOOTH

#else  // 自动瞳孔缩放：非阻塞状态机，每帧更新（替代阻塞式 split()）

  static bool     irisInMotion      = false;
  static int16_t  irisOld           = (IRIS_MIN + IRIS_MAX) / 2;
  static int16_t  irisNew           = irisOld;
  static uint32_t irisMoveStartTime = 0L;
  static int32_t  irisMoveDuration  = 0L;

  uint32_t t = micros();
  int16_t  irisScale;

  if (irisInMotion) {
    int32_t dt = t - irisMoveStartTime;
    if (dt >= irisMoveDuration) {
      irisInMotion      = false;
      irisMoveDuration  = random(500000, 2000000);
      irisMoveStartTime = t;
      irisOld           = irisNew;
      irisScale         = irisNew;
    } else {
      int16_t e = ease[255 * dt / irisMoveDuration] + 1;
      irisScale = irisOld + (((irisNew - irisOld) * e) / 256);
    }
  } else {
    irisScale = irisOld;
    if ((int32_t)(t - irisMoveStartTime) >= irisMoveDuration) {
      irisNew           = random(IRIS_MIN, IRIS_MAX);
      irisMoveDuration  = random(2000000, 5000000);
      irisMoveStartTime = t;
      irisInMotion      = true;
    }
  }

  frame(irisScale);

#endif // LIGHT_PIN
}

// 眼睛动画 --------------------------------------------------------------

#ifdef AUTOBLINK
uint32_t timeOfLastBlink = 0L, timeToNextBlink = 0L;
#endif

// 处理单帧（单眼）的运动、眨眼与渲染
void frame(uint16_t iScale) // 虹膜缩放值（0-1023）
{
  static uint32_t frames   = 0; // 帧计数，用于 FPS
  static uint8_t  eyeIndex = 0; // 当前渲染的眼睛索引
  int16_t         eyeX, eyeY;
  uint32_t        t = micros(); // 本帧开始时刻

  ++frames;
  if (frames == 32 || !(frames & 255)) { // 第 32 帧及之后每 256 帧打印一次
    float elapsed = (millis() - startTime) / 1000.0;
    if (elapsed) {
      Serial.print("FPS: ");
      Serial.println((uint16_t)(frames / elapsed)); // 输出 FPS
    }
  }

  if (++eyeIndex >= NUM_EYES) eyeIndex = 0; // 轮流渲染各眼，每次 frame 只画一只

  // X/Y 眼球移动 --------------------------------------------------------

#if defined(JOYSTICK_X_PIN) && (JOYSTICK_X_PIN >= 0) && \
    defined(JOYSTICK_Y_PIN) && (JOYSTICK_Y_PIN >= 0)

  // 摇杆输入，限制在圆形范围内
  int16_t dx, dy;
  int32_t d;
  eyeX = analogRead(JOYSTICK_X_PIN);
  eyeY = analogRead(JOYSTICK_Y_PIN);
#ifdef JOYSTICK_X_FLIP
  eyeX = 1023 - eyeX;
#endif
#ifdef JOYSTICK_Y_FLIP
  eyeY = 1023 - eyeY;
#endif
  dx = (eyeX * 2) - 1023; // ADC 中心约 511.5，×2 后范围 -1023~+1023
  dy = (eyeY * 2) - 1023;
  if ((d = (dx * dx + dy * dy)) > (1023 * 1023)) { // 超出圆形范围则裁剪
    d    = (int32_t)sqrt((float)d);
    eyeX = ((dx * 1023 / d) + 1023) / 2;
    eyeY = ((dy * 1023 / d) + 1023) / 2;
  }

#else // 自动随机眼球移动

  // 周期性地移动到新的随机位置，速度随机，停留随机时长后再移动
  static bool  eyeInMotion      = false;
  static int16_t  eyeOldX = 512, eyeOldY = 512, eyeNewX = 512, eyeNewY = 512;
  static uint32_t eyeMoveStartTime = 0L;
  static int32_t  eyeMoveDuration  = 0L;

  int32_t dt = t - eyeMoveStartTime;      // 距上次眼球事件经过的微秒数
  if (eyeInMotion) {                      // 正在移动？
    if (dt >= eyeMoveDuration) {          // 时间到，到达目标
      eyeInMotion      = false;
      eyeMoveDuration  = random(3000000); // 停留 0~3 秒
      eyeMoveStartTime = t;
      eyeX = eyeOldX = eyeNewX;
      eyeY = eyeOldY = eyeNewY;
    } else { // 移动未完成，按缓动曲线插值
      int16_t e = ease[255 * dt / eyeMoveDuration] + 1;
      eyeX = eyeOldX + (((eyeNewX - eyeOldX) * e) / 256);
      eyeY = eyeOldY + (((eyeNewY - eyeOldY) * e) / 256);
    }
  } else {                                // 眼球静止
    eyeX = eyeOldX;
    eyeY = eyeOldY;
    if (dt > eyeMoveDuration) {           // 停留结束，开始新移动
      int16_t  dx, dy;
      uint32_t d;
      do {                                // 在圆内随机选目标点
        eyeNewX = random(1024);
        eyeNewY = random(1024);
        dx      = (eyeNewX * 2) - 1023;
        dy      = (eyeNewY * 2) - 1023;
      } while ((d = (dx * dx + dy * dy)) > (1023 * 1023));
      eyeMoveDuration  = random(72000, 144000); // 移动约 1/14 ~ 1/7 秒
      eyeMoveStartTime = t;
      eyeInMotion      = true;
    }
  }
#endif // JOYSTICK_X_PIN 等

  // 眨眼 ----------------------------------------------------------------
#ifdef AUTOBLINK
  // 自动眨眼：开始时刻与持续时间随机
  if ((t - timeOfLastBlink) >= timeToNextBlink) {
    timeOfLastBlink = t;
    uint32_t blinkDuration = random(36000, 72000); // 约 1/28 ~ 1/14 秒
    for (uint8_t e = 0; e < NUM_EYES; e++) {
      if (eye[e].blink.state == NOBLINK) {
        eye[e].blink.state     = ENBLINK;
        eye[e].blink.startTime = t;
        eye[e].blink.duration  = blinkDuration;
      }
    }
    timeToNextBlink = blinkDuration * 3 + random(4000000);
  }
#endif

  if (eye[eyeIndex].blink.state) { // 当前正在眨眼？
    if ((t - eye[eyeIndex].blink.startTime) >= eye[eyeIndex].blink.duration) {
      if ((eye[eyeIndex].blink.state == ENBLINK) && (
#if defined(BLINK_PIN) && (BLINK_PIN >= 0)
            (digitalRead(BLINK_PIN) == LOW) ||
#endif
            ((eyeInfo[eyeIndex].wink >= 0) &&
             digitalRead(eyeInfo[eyeIndex].wink) == LOW) )) {
        // 按钮仍按住：保持闭眼，不进入下一阶段
      } else {
        if (++eye[eyeIndex].blink.state > DEBLINK) {
          eye[eyeIndex].blink.state = NOBLINK;
        } else { // ENBLINK -> DEBLINK
          eye[eyeIndex].blink.duration *= 2; // 睁眼阶段时长为闭眼的一半
          eye[eyeIndex].blink.startTime = t;
        }
      }
    }
  } else { // 未在眨眼，检测按钮
#if defined(BLINK_PIN) && (BLINK_PIN >= 0)
    if (digitalRead(BLINK_PIN) == LOW) {
      uint32_t blinkDuration = random(36000, 72000);
      for (uint8_t e = 0; e < NUM_EYES; e++) {
        if (eye[e].blink.state == NOBLINK) {
          eye[e].blink.state     = ENBLINK;
          eye[e].blink.startTime = t;
          eye[e].blink.duration  = blinkDuration;
        }
      }
    } else
#endif
      if ((eyeInfo[eyeIndex].wink >= 0) &&
          (digitalRead(eyeInfo[eyeIndex].wink) == LOW)) { // 单眼 wink
        eye[eyeIndex].blink.state     = ENBLINK;
        eye[eyeIndex].blink.startTime = t;
        eye[eyeIndex].blink.duration  = random(45000, 90000);
      }
  }

  // 将运动、眨眼、瞳孔缩放合成为可渲染参数 --------------------------------

  // 将逻辑坐标 0-1023 映射为 drawEye 使用的像素偏移
  eyeX = map(eyeX, 0, 1023, 0, SCLERA_WIDTH  - 128);
  eyeY = map(eyeY, 0, 1023, 0, SCLERA_HEIGHT - 128);

  // 双眼略微内聚，模拟对视距离上的会聚感
  if (NUM_EYES > 1) {
    if (eyeIndex == 1) eyeX += 4;
    else eyeX -= 4;
  }
  if (eyeX > (SCLERA_WIDTH - 128)) eyeX = (SCLERA_WIDTH - 128);

  // 上眼皮随瞳孔位置略微开合（TRACKING）
  static uint8_t uThreshold = 0;
  uint8_t        lThreshold, n;
#ifdef TRACKING
  int16_t sampleX = SCLERA_WIDTH  / 2 - (eyeX / 2),
          sampleY = SCLERA_HEIGHT / 2 - (eyeY + IRIS_HEIGHT / 4);
  if (sampleY < 0) n = 0;
  else n = (ramLidStore[SCREEN_WIDTH * SCREEN_HEIGHT + sampleY * SCREEN_WIDTH + sampleX] +
            ramLidStore[SCREEN_WIDTH * SCREEN_HEIGHT + sampleY * SCREEN_WIDTH + (SCREEN_WIDTH - 1 - sampleX)]) / 2;
  uThreshold = (uThreshold * 3 + n) / 4;
  lThreshold = 254 - uThreshold;         // 下眼皮受上眼皮牵连
#else
  uThreshold = lThreshold = 0; // 不跟踪时眼皮完全睁开（除非眨眼）
#endif

  // 按当前眨眼进度缩放眼皮阈值
  if (eye[eyeIndex].blink.state) {
    uint32_t s = (t - eye[eyeIndex].blink.startTime);
    if (s >= eye[eyeIndex].blink.duration) s = 255;
    else s = 255 * s / eye[eyeIndex].blink.duration;
    s          = (eye[eyeIndex].blink.state == DEBLINK) ? 1 + s : 256 - s;
    n          = (uThreshold * s + 254 * (257 - s)) / 256;
    lThreshold = (lThreshold * s + 254 * (257 - s)) / 256;
  } else {
    n          = uThreshold;
  }

  drawEye(eyeIndex, iScale, eyeX, eyeY, n, lThreshold);

  if (eyeIndex == (NUM_EYES - 1)) {
    user_loop(); // 最后一只眼睛画完后调用用户代码
  }
}
