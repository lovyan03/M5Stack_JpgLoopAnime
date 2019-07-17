#pragma GCC optimize ("O3")

#include <M5Stack.h>
//#include <M5StackUpdater.h>     // https://github.com/tobozo/M5Stack-SD-Updater/
#include <esp_heap_alloc_caps.h>
#include <vector>
#include "src/MainClass.h"
#include "src/DMADrawer.h"

MainClass main;

// ここで画像ファイルのディレクトリ名を指定する
static const char imageDir[] = {"/image_dirname"};


std::vector<uint8_t*> fbuf;
std::vector<int32_t> fbufsize;
uint32_t fpsCount = 0, fpsSec = 0;

void setup() {

  M5.begin();

/*
  if(digitalRead(BUTTON_A_PIN) == 0) {
     Serial.println("Will Load menu binary");
     updateFromFS(SD);
     ESP.restart();
  }
*/

  main.setup( M5.Lcd.width()
            , M5.Lcd.height()
            , SPI_FREQUENCY
            , TFT_MOSI
            , TFT_MISO
            , TFT_SCLK
            , TFT_CS
            , TFT_DC
            );

  Serial.println(imageDir);
  File root = SD.open(imageDir);
  File file = root.openNextFile();
  while (file) {
    fbufsize.push_back(file.size());
    fbuf.push_back((uint8_t*)pvPortMallocCaps(fbufsize.back(), MALLOC_CAP_DEFAULT));
    file.read(fbuf.back(), fbufsize.back());
    file = root.openNextFile();
  }
  SD.end();
}

void loop() {
  for (int i = 0; i < fbuf.size(); i++) {
//      M5.Lcd.drawJpg(fbuf[i], fbufsize[i]);
    main.drawJpg(fbuf[i], fbufsize[i]);
    fpsCount++;
    if (fpsSec != millis() / 1000) {
      fpsSec = millis() / 1000;
      Serial.printf("fps:%d\r\n",fpsCount);
      fpsCount = 0;
    }
  }
/*
  for (int i = fbuf.size() - 2; i != 0; i--) {
//      M5.Lcd.drawJpg(fbuf[i], fbufsize[i]);
    main.drawJpg(fbuf[i], fbufsize[i]);
    fpsCount++;
    if (fpsSec != millis() / 1000) {
      fpsSec = millis() / 1000;
      Serial.printf("fps:%d\r\n",fpsCount);
      fpsCount = 0;
    }
  }
*/
}
