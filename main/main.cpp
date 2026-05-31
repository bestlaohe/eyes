#include <Arduino.h>

#include "eyes_common.h"

TFT_eSPI tft;

uint16_t pbuffer[BUFFERS][BUFFER_SIZE];
bool dmaBuf = 0;

EyeState eye[NUM_EYES];

uint32_t startTime;



static void backlight_on(void) {

#if defined(DISPLAY_BACKLIGHT) && (DISPLAY_BACKLIGHT >= 0)

  pinMode(DISPLAY_BACKLIGHT, OUTPUT);

  digitalWrite(DISPLAY_BACKLIGHT, HIGH);

  analogWrite(DISPLAY_BACKLIGHT, BACKLIGHT_MAX);

#endif

}



void setup(void) {

  Serial.begin(115200);

  Serial.println("开始");



  backlight_on();



  user_setup();

  initEyes();



  Serial.println("初始化显示器");

#if defined(TFT_RST) && (TFT_RST >= 0)

  pinMode(TFT_RST, OUTPUT);

  digitalWrite(TFT_RST, HIGH);

  delay(50);

  digitalWrite(TFT_RST, LOW);

  delay(50);

  digitalWrite(TFT_RST, HIGH);

  delay(120);

#endif

  tft.init();



#ifdef USE_DMA

  tft.initDMA();

#endif



  digitalWrite(eye[0].tft_cs, HIGH);

  if (NUM_EYES > 1) digitalWrite(eye[1].tft_cs, HIGH);



  for (uint8_t e = 0; e < NUM_EYES; e++) {

    digitalWrite(eye[e].tft_cs, LOW);

    tft.setRotation(eyeInfo[e].rotation);

    tft.fillScreen(TFT_BLACK);

    digitalWrite(eye[e].tft_cs, HIGH);

  }



  Serial.printf("FW %s %s\n", __DATE__, __TIME__);
  Serial.println("背光已打开，显示初始化完成");

  backlight_on();

  startTime = millis();
}



void loop(void) {
  updateEye();
}
