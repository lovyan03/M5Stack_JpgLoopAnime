#pragma GCC optimize ("O3")

#include <M5Stack.h>
//#include <M5StackUpdater.h>     // https://github.com/tobozo/M5Stack-SD-Updater/
#include <esp_heap_alloc_caps.h>
#include <vector>
#include "src/MainClass.h"
#include "src/DMADrawer.h"

MainClass main;

// ここで画像ファイルのディレクトリ名を指定する
std::vector<String> imageDirs = {"/image_dirname1", "/image_dirname2"};

uint8_t dirIndex = 0;
std::vector<uint8_t*> fbuf;
std::vector<int32_t> fbufsize;
uint32_t fpsCount = 0, fpsSec = 0;

bool loadImages(const String& path)
{
  bool res = false;
  for (int i = 0; i < fbuf.size(); i++) free(fbuf[i]);
  fbuf.clear();
  fbufsize.clear();
  Serial.println(path);
  File root = SD.open(path);
  File file = root.openNextFile();
  uint8_t* tmp;
  while (file) {
    tmp = (uint8_t*)pvPortMallocCaps(file.size(), MALLOC_CAP_DEFAULT);
    if (tmp > 0) {
      file.read(tmp, file.size());
      fbufsize.push_back(file.size());
      fbuf.push_back(tmp);
      res = true;
    }
    file = root.openNextFile();
  }
  return res;
}

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

  loadImages(imageDirs[dirIndex]);
}

void loop() {
  for (int i = 0; i < fbuf.size(); i++) {
    M5.update();
    if (M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
      delay(100);
      do {
        dirIndex = (dirIndex + (M5.BtnC.wasPressed() ? 1 : M5.BtnB.wasPressed() ? imageDirs.size() - 1 : 0)) % imageDirs.size();
      } while (!loadImages(imageDirs[dirIndex]));
      M5.Lcd.fillRect(0,0,M5.Lcd.width(),M5.Lcd.height(),0);
      break;
    }
//  M5.Lcd.drawJpg(fbuf[i], fbufsize[i]);
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
