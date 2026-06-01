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
#include "esp_task_wdt.h"
#include "eyes_common.h"

#if !defined(LIGHT_PIN) || (LIGHT_PIN < 0)
// 自动瞳孔缩放：用分形行为模拟瞳孔的主要反应和持续微调
uint16_t oldIris = (IRIS_MIN + IRIS_MAX) / 2, newIris;
#endif

// 初始化眼睛 ---------------------------------------------------------
void initEyes(void)
{
  Serial.println("初始化眼睛对象");

  // 根据 config.h 中的 eyeInfo 列表初始化各眼对象
  for (uint8_t e = 0; e < NUM_EYES; e++) {
    Serial.print("创建显示器 #"); Serial.println(e);

    eye[e].tft_cs      = eyeInfo[e].select;
    eye[e].blink.state = NOBLINK;
    eye[e].xposition   = eyeInfo[e].xposition;
    eye[e].yposition   = eyeInfo[e].yposition;

    pinMode(eye[e].tft_cs, OUTPUT);
    digitalWrite(eye[e].tft_cs, HIGH);

    // 若定义了单眼 wink 引脚，则配置为上拉输入
    if (eyeInfo[e].wink >= 0) pinMode(eyeInfo[e].wink, INPUT_PULLUP);
  }

#if defined(BLINK_PIN) && (BLINK_PIN >= 0)
  pinMode(BLINK_PIN, INPUT_PULLUP); // 双眼共用的眨眼按钮引脚
#endif
}

// 更新眼睛 --------------------------------------------------------------
void updateEye (void)
{
#if defined(LIGHT_PIN) && (LIGHT_PIN >= 0) // 交互式虹膜（光线/电位器控制）

  int16_t v = analogRead(LIGHT_PIN);       // 电位器或光敏电阻原始读数
#ifdef LIGHT_PIN_FLIP
  v = 1023 - v;                            // 反转传感器读数
#endif
  if (v < LIGHT_MIN)      v = LIGHT_MIN; // 限制光线传感器范围
  else if (v > LIGHT_MAX) v = LIGHT_MAX;
  v -= LIGHT_MIN;  // 归一化到 0 .. (LIGHT_MAX - LIGHT_MIN)
#ifdef LIGHT_CURVE  // 对传感器输入应用 gamma 曲线？
  v = (int16_t)(pow((double)v / (double)(LIGHT_MAX - LIGHT_MIN),
                    LIGHT_CURVE) * (double)(LIGHT_MAX - LIGHT_MIN));
#endif
  // 映射到虹膜尺寸范围（IRIS_MAX 对应最亮时的瞳孔大小）
  v = map(v, 0, (LIGHT_MAX - LIGHT_MIN), IRIS_MAX, IRIS_MIN);
#ifdef IRIS_SMOOTH // 滤波输入（平滑运动）
  static int16_t irisValue = (IRIS_MIN + IRIS_MAX) / 2;
  irisValue = ((irisValue * 15) + v) / 16;
  frame(irisValue);
#else // 无滤波（即时响应）
  frame(v);
#endif // IRIS_SMOOTH

#else  // 自动瞳孔缩放 —— 调用递归 split 函数
Serial.println("2222");
  newIris = random(IRIS_MIN, IRIS_MAX);
  split(oldIris, newIris, micros(), 10000000L, IRIS_MAX - IRIS_MIN);
  oldIris = newIris;
  Serial.println("在动");
#endif // LIGHT_PIN
}

// 渲染单眼 --------------------------------------------------------------
void drawEye(uint8_t e, uint32_t iScale, uint32_t scleraX, uint32_t scleraY,
             uint32_t uT, uint32_t lT) {
  uint32_t screenX, screenY, scleraXsave;
  int32_t  irisX, irisY;
  uint32_t p, a, d;
  uint32_t pixels = 0;

  digitalWrite(eye[e].tft_cs, LOW);
  tft.startWrite();
  tft.setAddrWindow(eye[e].xposition, eye[e].yposition, 128, 128);

  scleraXsave = scleraX;
  irisY       = scleraY - (SCLERA_HEIGHT - IRIS_HEIGHT) / 2;

  // 双屏时左右眼睑贴图镜像
  uint16_t lidX = 0;
  int16_t  dlidX = -1;
  if (e) dlidX = 1;
  for (screenY = 0; screenY < SCREEN_HEIGHT; screenY++, scleraY++, irisY++) {
    scleraX = scleraXsave;
    irisX   = scleraXsave - (SCLERA_WIDTH - IRIS_WIDTH) / 2;
    if (e) lidX = 0;
    else lidX = SCREEN_WIDTH - 1;
    for (screenX = 0; screenX < SCREEN_WIDTH;
         screenX++, scleraX++, irisX++, lidX += dlidX) {
      if ((pgm_read_byte(lower + screenY * SCREEN_WIDTH + lidX) <= lT) ||
          (pgm_read_byte(upper + screenY * SCREEN_WIDTH + lidX) <= uT)) {
        p = 0;  // 被眼睑遮挡
      } else if ((irisY < 0) || (irisY >= IRIS_HEIGHT) ||
                 (irisX < 0) || (irisX >= IRIS_WIDTH)) {
        p = pgm_read_word(sclera + scleraY * SCLERA_WIDTH + scleraX);  // 巩膜区域
      } else {
        p = pgm_read_word(polar + irisY * IRIS_WIDTH + irisX);  // 极坐标角度/距离
        d = (iScale * (p & 0x7F)) / 128;                // 距离 (Y)
        if (d < IRIS_MAP_HEIGHT) {                      // 在虹膜范围内
          a = (IRIS_MAP_WIDTH * (p >> 7)) / 512;        // 角度 (X)
          p = pgm_read_word(iris + d * IRIS_MAP_WIDTH + a);
        } else {
          p = pgm_read_word(sclera + scleraY * SCLERA_WIDTH + scleraX);
        }
      }
      *(&pbuffer[dmaBuf][0] + pixels++) = p >> 8 | p << 8;

      if (pixels >= BUFFER_SIZE) {
        yield();
#ifdef USE_DMA
        tft.pushPixelsDMA(&pbuffer[dmaBuf][0], pixels);
        dmaBuf = !dmaBuf;
#else
        tft.pushPixels(pbuffer[0], pixels);
#endif
        pixels = 0;
      }
    }
  }

  if (pixels) {
#ifdef USE_DMA
    tft.pushPixelsDMA(&pbuffer[dmaBuf][0], pixels);
#else
    tft.pushPixels(pbuffer[0], pixels);
#endif
  }
  tft.endWrite();
  digitalWrite(eye[e].tft_cs, HIGH);
}

// 眼睛动画 --------------------------------------------------------------

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

#ifdef AUTOBLINK
uint32_t timeOfLastBlink = 0L, timeToNextBlink = 0L;
#endif

// 处理单帧左/右眼的运动
void frame(uint16_t iScale) // 虹膜缩放 (IRIS_MIN..IRIS_MAX)
{
  static uint32_t frames   = 0;
  static uint8_t  eyeIndex = 0; // eye[] 数组索引
  int16_t         eyeX, eyeY;
  uint32_t        t = micros(); // 函数开始时的时刻

  if (!(++frames & 255)) { // 每 256 帧打印一次 FPS
    float elapsed = (millis() - startTime) / 1000.0;
    if (elapsed) Serial.println((uint16_t)(frames / elapsed));
  }

  if (++eyeIndex >= NUM_EYES) eyeIndex = 0; // 轮流渲染各眼，每帧一只

  // X/Y 运动

#if defined(JOYSTICK_X_PIN) && (JOYSTICK_X_PIN >= 0) && \
    defined(JOYSTICK_Y_PIN) && (JOYSTICK_Y_PIN >= 0)

  // 从摇杆读取 X/Y，限制在圆形范围内
  int16_t dx, dy;
  int32_t d;
  eyeX = analogRead(JOYSTICK_X_PIN); // 原始 X/Y 读数（未裁剪）
  eyeY = analogRead(JOYSTICK_Y_PIN);
#ifdef JOYSTICK_X_FLIP
  eyeX = 1023 - eyeX;
#endif
#ifdef JOYSTICK_Y_FLIP
  eyeY = 1023 - eyeY;
#endif
  dx = (eyeX * 2) - 1023; // ADC 中心约 511.5，放大后中心为 0
  dy = (eyeY * 2) - 1023; // 范围 -1023 .. +1023
  if ((d = (dx * dx + dy * dy)) > (1023 * 1023)) { // 超出圆形
    d    = (int32_t)sqrt((float)d);               // 到中心的距离
    eyeX = ((dx * 1023 / d) + 1023) / 2;          // 裁剪到圆边
    eyeY = ((dy * 1023 / d) + 1023) / 2;          // 映射回 0-1023
  }

#else // 自动 X/Y 眼球运动
  // 周期性移动到新的随机点，随机速度，停留随机时长后再移动

  static bool  eyeInMotion      = false;
  static int16_t  eyeOldX = 512, eyeOldY = 512, eyeNewX = 512, eyeNewY = 512;
  static uint32_t eyeMoveStartTime = 0L;
  static int32_t  eyeMoveDuration  = 0L;

  int32_t dt = t - eyeMoveStartTime;      // 距上次眼球事件的微秒数
  if (eyeInMotion) {                      // 正在移动？
    if (dt >= eyeMoveDuration) {          // 时间到？已到达目标
      eyeInMotion      = false;           // 停止移动
      eyeMoveDuration  = random(3000000); // 停留 0-3 秒
      eyeMoveStartTime = t;               // 记录停留开始时刻
      eyeX = eyeOldX = eyeNewX;           // 保存位置
      eyeY = eyeOldY = eyeNewY;
    } else { // 移动尚未完成 —— 插值计算当前位置
      int16_t e = ease[255 * dt / eyeMoveDuration] + 1;   // 缓动曲线
      eyeX = eyeOldX + (((eyeNewX - eyeOldX) * e) / 256); // 插值 X
      eyeY = eyeOldY + (((eyeNewY - eyeOldY) * e) / 256); // 插值 Y
    }
  } else {                                // 眼球静止
    eyeX = eyeOldX;
    eyeY = eyeOldY;
    if (dt > eyeMoveDuration) {           // 停留结束？开始新移动
      int16_t  dx, dy;
      uint32_t d;
      do {                                // 在圆内随机选目标点
        eyeNewX = random(1024);
        eyeNewY = random(1024);
        dx      = (eyeNewX * 2) - 1023;
        dy      = (eyeNewY * 2) - 1023;
      } while ((d = (dx * dx + dy * dy)) > (1023 * 1023)); // 直到落在圆内
      eyeMoveDuration  = random(72000, 144000); // 约 1/14 - 1/7 秒
      eyeMoveStartTime = t;               // 记录移动开始时刻
      eyeInMotion      = true;            // 下一帧开始移动
    }
  }
#endif // JOYSTICK_X_PIN 等

  // 眨眼
#ifdef AUTOBLINK
  // 与自动眼球运动类似 —— 随机眨眼时刻和时长
  if ((t - timeOfLastBlink) >= timeToNextBlink) { // 开始新眨眼？
    timeOfLastBlink = t;
    uint32_t blinkDuration = random(36000, 72000); // 约 1/28 - 1/14 秒
    // 为双眼设置眨眼时长（若尚未在 wink）
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
    // 检查当前眨眼状态是否超时
    if ((t - eye[eyeIndex].blink.startTime) >= eye[eyeIndex].blink.duration) {
      // 是 —— 推进状态，除非...
      if ((eye[eyeIndex].blink.state == ENBLINK) && ( // 正在闭眼且...
#if defined(BLINK_PIN) && (BLINK_PIN >= 0)
            (digitalRead(BLINK_PIN) == LOW) ||           // 眨眼/wink 按钮仍按住
#endif
            ((eyeInfo[eyeIndex].wink >= 0) &&
             digitalRead(eyeInfo[eyeIndex].wink) == LOW) )) {
        // 暂不推进状态 —— 眼睛被按住保持闭合
      } else { // 无按钮或其他状态
        if (++eye[eyeIndex].blink.state > DEBLINK) { // 睁眼完成？
          eye[eyeIndex].blink.state = NOBLINK;      // 不再眨眼
        } else { // 从 ENBLINK 进入 DEBLINK
          eye[eyeIndex].blink.duration *= 2; // DEBLINK 速度为 ENBLINK 的一半
          eye[eyeIndex].blink.startTime = t;
        }
      }
    }
  } else { // 未在眨眼 —— 检测按钮
#if defined(BLINK_PIN) && (BLINK_PIN >= 0)
    if (digitalRead(BLINK_PIN) == LOW) {
      // 手动眨眼时长与自动眨眼相同（随机）
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
          (digitalRead(eyeInfo[eyeIndex].wink) == LOW)) { // 单眼 wink！
        eye[eyeIndex].blink.state     = ENBLINK;
        eye[eyeIndex].blink.startTime = t;
        eye[eyeIndex].blink.duration  = random(45000, 90000);
      }
  }

  // 将运动、眨眼和虹膜缩放转换为可渲染参数

  // 将眼球 X/Y (0-1023) 映射为 drawEye() 使用的像素偏移
  eyeX = map(eyeX, 0, 1023, 0, SCLERA_WIDTH  - 128);
  eyeY = map(eyeY, 0, 1023, 0, SCLERA_HEIGHT - 128);

  // 水平微调使双眼略微内聚，模拟对话距离的注视
  if (NUM_EYES > 1) {
    if (eyeIndex == 1) eyeX += 4;
    else eyeX -= 4;
  }
  if (eyeX > (SCLERA_WIDTH - 128)) eyeX = (SCLERA_WIDTH - 128);

  // 眼睑用亮度阈值图渲染；同一贴图可用于上眼睑跟踪瞳孔
  static uint8_t uThreshold = 128;
  uint8_t        lThreshold, n;
#ifdef TRACKING
  int16_t sampleX = SCLERA_WIDTH  / 2 - (eyeX / 2), // 减弱 X 方向影响
          sampleY = SCLERA_HEIGHT / 2 - (eyeY + IRIS_HEIGHT / 4);
  // 眼睑略不对称，取两点平均
  if (sampleY < 0) n = 0;
  else            n = (pgm_read_byte(upper + sampleY * SCREEN_WIDTH + sampleX) +
                         pgm_read_byte(upper + sampleY * SCREEN_WIDTH + (SCREEN_WIDTH - 1 - sampleX))) / 2;
  uThreshold = (uThreshold * 3 + n) / 4; // 滤波平滑
  // 下眼睑受上眼睑张力上提
  lThreshold = 254 - uThreshold;
#else // 无跟踪 —— 除非眨眼，否则眼睑完全睁开
  uThreshold = lThreshold = 0;
#endif

  // 根据当前眨眼进度缩放上下眼睑阈值，使眨眼与瞳孔跟踪协同
  if (eye[eyeIndex].blink.state) { // 正在眨眼？
    uint32_t s = (t - eye[eyeIndex].blink.startTime);
    if (s >= eye[eyeIndex].blink.duration) s = 255;  // 眨眼已结束或超时
    else s = 255 * s / eye[eyeIndex].blink.duration; // 眨眼进行中
    s          = (eye[eyeIndex].blink.state == DEBLINK) ? 1 + s : 256 - s;
    n          = (uThreshold * s + 254 * (257 - s)) / 256;
    lThreshold = (lThreshold * s + 254 * (257 - s)) / 256;
  } else {
    n          = uThreshold;
  }

  drawEye(eyeIndex, iScale, eyeX, eyeY, n, lThreshold);

  if (eyeIndex == (NUM_EYES - 1)) {
    user_loop(); // 最后一只眼渲染完成后调用用户代码
  }
}

// 自动瞳孔缩放（无光敏电阻或电位器时）---------------------------------

#if !defined(LIGHT_PIN) || (LIGHT_PIN < 0)

// 用分形递归将瞳孔变化路径细分，模拟自然缩放

void split(
  int16_t  startValue, // 起始虹膜缩放值 (IRIS_MIN .. IRIS_MAX)
  int16_t  endValue,   // 结束虹膜缩放值
  uint32_t startTime,  // 起始时刻 micros()
  int32_t  duration,   // 总时长（微秒）
  int16_t  range) {    // 细分时允许的缩放值随机偏差

  if (range >= 8) {    // 限制递归深度
    range    /= 2;     // 范围和时间各减半
    duration /= 2;     // 在范围内随机取中点
    int16_t  midValue = (startValue + endValue - range) / 2 + random(range);
    uint32_t midTime  = startTime + duration;
    split(startValue, midValue, startTime, duration, range); // 前半段
    split(midValue  , endValue, midTime  , duration, range); // 后半段
  } else {             // 不再细分，执行瞳孔动画
    int32_t dt;        // 距运动开始的微秒数
    int16_t v;         // 中间插值
    Serial.println("micros");
    while ((dt = (micros() - startTime)) < duration) {
      v = startValue + (((endValue - startValue) * dt) / duration);
      if (v < IRIS_MIN)      v = IRIS_MIN; // 安全裁剪
      else if (v > IRIS_MAX) v = IRIS_MAX;
      Serial.printf("micros=%d\n", v);
      frame(v);
      Serial.println("micros");
    }
  }
}
#endif // !LIGHT_PIN
