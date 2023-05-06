/*----------------------------------------------------------------------------/
  M5Stack JpgLoopAnime  

Original Source:  
 https://github.com/lovyan03/M5Stack_JpgLoopAnime/  

Licence:  
 [MIT](https://github.com/lovyan03/M5Stack_JpgLoopAnime/blob/master/LICENSE)  

Author:  
 [lovyan03](https://twitter.com/lovyan03)  

/----------------------------------------------------------------------------*/

#include <SD.h>
#include <M5Unified.h>
// For SD-Updater defineds
# define HAS_M5_API
# define SDU_USE_DISPLAY
# define HAS_LGFX
# define SDU_Sprite LGFX_Sprite
# define SDU_DISPLAY_TYPE M5GFX*
# define SDU_DISPLAY_OBJ_PTR &M5.Display
#include <M5StackUpdater.h>     // https://github.com/tobozo/M5Stack-SD-Updater/
#include <esp_heap_caps.h>
#include <vector>
#include "src/MainClass.h"
#include "src/images.h"

static auto& lcd = M5.Display;
static MainClass mainClass;

// ここで画像ファイルのディレクトリ名を指定する
std::vector<String> imageDirs = {"/image_dirname1", "/image_dirname2"};

uint_fast8_t dirIndex = 0;
std::vector<const uint8_t*> fbuf;
std::vector<int32_t> fbufsize;
uint32_t fpsCount = 0, fpsSec = 0;

bool loadImages(const String& path)
{
  bool res = false;
  if (!fbuf.empty() && fbuf[0] != image_000_jpg) {
    for (int i = 0; i < fbuf.size(); i++) free(const_cast<uint8_t*>(fbuf[i]));
  }
  fbuf.clear();
  fbufsize.clear();
  Serial.println(path);
  File root = SD.open(path);
  File file = root.openNextFile();
  uint8_t* tmp;
  while (file) {
    tmp = (uint8_t*)heap_caps_malloc(file.size(), MALLOC_CAP_DEFAULT);
    if (tmp) {
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

#if defined ( __M5STACKUPDATER_H )
  checkSDUpdater(SD);
#endif

  mainClass.setup(&lcd);

//  loadImages(imageDirs[dirIndex]);
  fbuf.clear();
  fbufsize.clear();
  fbufsize.push_back(image_000_jpg_len); fbuf.push_back(image_000_jpg);
  fbufsize.push_back(image_001_jpg_len); fbuf.push_back(image_001_jpg);
  fbufsize.push_back(image_002_jpg_len); fbuf.push_back(image_002_jpg);
  fbufsize.push_back(image_003_jpg_len); fbuf.push_back(image_003_jpg);
  fbufsize.push_back(image_004_jpg_len); fbuf.push_back(image_004_jpg);
  fbufsize.push_back(image_005_jpg_len); fbuf.push_back(image_005_jpg);
  fbufsize.push_back(image_006_jpg_len); fbuf.push_back(image_006_jpg);
  fbufsize.push_back(image_007_jpg_len); fbuf.push_back(image_007_jpg);
  fbufsize.push_back(image_008_jpg_len); fbuf.push_back(image_008_jpg);
  fbufsize.push_back(image_009_jpg_len); fbuf.push_back(image_009_jpg);
  fbufsize.push_back(image_010_jpg_len); fbuf.push_back(image_010_jpg);
  fbufsize.push_back(image_011_jpg_len); fbuf.push_back(image_011_jpg);
  fbufsize.push_back(image_012_jpg_len); fbuf.push_back(image_012_jpg);
  fbufsize.push_back(image_013_jpg_len); fbuf.push_back(image_013_jpg);
  fbufsize.push_back(image_014_jpg_len); fbuf.push_back(image_014_jpg);
  fbufsize.push_back(image_015_jpg_len); fbuf.push_back(image_015_jpg);
  fbufsize.push_back(image_016_jpg_len); fbuf.push_back(image_016_jpg);
  fbufsize.push_back(image_017_jpg_len); fbuf.push_back(image_017_jpg);
  fbufsize.push_back(image_018_jpg_len); fbuf.push_back(image_018_jpg);
  fbufsize.push_back(image_019_jpg_len); fbuf.push_back(image_019_jpg);
  fbufsize.push_back(image_020_jpg_len); fbuf.push_back(image_020_jpg);
  fbufsize.push_back(image_021_jpg_len); fbuf.push_back(image_021_jpg);
  fbufsize.push_back(image_022_jpg_len); fbuf.push_back(image_022_jpg);
  fbufsize.push_back(image_023_jpg_len); fbuf.push_back(image_023_jpg);
  fbufsize.push_back(image_024_jpg_len); fbuf.push_back(image_024_jpg);
  fbufsize.push_back(image_025_jpg_len); fbuf.push_back(image_025_jpg);
  fbufsize.push_back(image_026_jpg_len); fbuf.push_back(image_026_jpg);
  fbufsize.push_back(image_027_jpg_len); fbuf.push_back(image_027_jpg);
  fbufsize.push_back(image_028_jpg_len); fbuf.push_back(image_028_jpg);
  fbufsize.push_back(image_029_jpg_len); fbuf.push_back(image_029_jpg);
  fbufsize.push_back(image_030_jpg_len); fbuf.push_back(image_030_jpg);

  lcd.startWrite();
}

void loop() {
//auto us = micros();
  for (int i = 0; i < fbuf.size(); i++) {
    M5.update();
    if (M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
      delay(10);
      lcd.endWrite();
      do {
        dirIndex = (dirIndex + (M5.BtnC.wasPressed() ? 1 : M5.BtnB.wasPressed() ? imageDirs.size() - 1 : 0)) % imageDirs.size();
      } while (!loadImages(imageDirs[dirIndex]));
      lcd.startWrite();
      lcd.fillRect(0,0,lcd.width(),lcd.height(),0);
      break;
    }
//    lcd.drawJpg(fbuf[i], fbufsize[i]);
    mainClass.drawJpg(fbuf[i], fbufsize[i]);
//*
    fpsCount++;
    if (fpsSec != millis() / 1000) {
      fpsSec = millis() / 1000;
      Serial.printf("fps:%d\r\n",fpsCount);
      fpsCount = 0;
    }
//*/
  }

  for (int i = fbuf.size() - 2; i != 0; i--) {
//    lcd.drawJpg(fbuf[i], fbufsize[i]);
    mainClass.drawJpg(fbuf[i], fbufsize[i]);
//*
    fpsCount++;
    if (fpsSec != millis() / 1000) {
      fpsSec = millis() / 1000;
      Serial.printf("fps:%d\r\n",fpsCount);
      fpsCount = 0;
    }
//*/
  }
//Serial.printf("%d usec\r\n", (int)(micros() - us));
}
