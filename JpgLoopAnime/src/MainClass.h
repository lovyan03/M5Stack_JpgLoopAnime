#ifndef _MAINCLASS_H_
#define _MAINCLASS_H_

#pragma GCC optimize ("O3")

#include <esp_heap_caps.h>

#include <LovyanGFX.hpp>  // https://github.com/lovyan03/LovyanGFX/

#include "tjpgdClass.h"

class MainClass
{
public:
  bool setup(LGFX* lcd)
  {
    Serial.println("MainClass setup.");

    _lcd = lcd;
    _lcd_width = lcd->width();
    _lcd_height = lcd->height();

    for (int i = 0; i < 2; ++i) _dmabufs[i] = (uint16_t*)heap_caps_malloc(_lcd_width * 16 * 2, MALLOC_CAP_DMA);

    _dmabuf = _dmabufs[0];

    _jdec.multitask_begin();

    return true;
  }

  bool drawJpg(const uint8_t* buf, int32_t len) {
    _filebuf = buf;
    _fileindex = 0;
    _remain = len;
    TJpgD::JRESULT jres = _jdec.prepare(jpgRead, this);
    if (jres != TJpgD::JDR_OK) {
      Serial.printf("prepare failed! %d\r\n", jres);
      return false;
    }

    _jpg_x = (_lcd_width - _jdec.width) / 2;
    _jpg_y = (_lcd_height- _jdec.height) / 2;
    jres = _jdec.decomp_multitask(jpgWrite, jpgWriteRow);
    if (jres != TJpgD::JDR_OK) {
      Serial.printf("decomp failed! %d\r\n", jres);
      return false;
    }
    return true;
  }

private:
  LGFX* _lcd;
  uint16_t* _dmabufs[2];
  uint16_t* _dmabuf;
  TJpgD _jdec;

  int32_t _remain = 0;
  const uint8_t* _filebuf;
  uint32_t _fileindex;

  uint_fast16_t _lcd_width;
  uint_fast16_t _lcd_height;
  uint_fast16_t _jpg_x;
  uint_fast16_t _jpg_y;

  static uint32_t jpgRead(TJpgD *jdec, uint8_t *buf, uint32_t len) {
    MainClass* me = (MainClass*)jdec->device;
    if (len > me->_remain)  len = me->_remain;
    if (buf) {
      memcpy(buf, (const uint8_t *)me->_filebuf + me->_fileindex, len);
    }
    me->_fileindex += len;
    me->_remain -= len;
    return len;
  }

  static uint32_t jpgWrite(TJpgD *jdec, void *bitmap, TJpgD::JRECT *rect) {
    MainClass* me = (MainClass*)jdec->device;
    uint16_t *p = me->_dmabuf;
    uint16_t *data = (uint16_t*)bitmap;
    int_fast16_t width = jdec->width;
    int_fast16_t x = rect->left;
    int_fast16_t y = rect->top;
    int_fast8_t w = rect->right + 1 - x;
    int_fast8_t h = rect->bottom + 1 - y;

    p += x;
    do {
      memcpy(p, data, w * 2);
      data += w;
      p += width;
    } while (--h);

    return 1;
  }

  static uint32_t jpgWriteRow(TJpgD *jdec, uint32_t y, uint32_t h) {
    static int flip = 0;
    MainClass* me = (MainClass*)jdec->device;
    if (y == 0)
      me->_lcd->setAddrWindow(me->_jpg_x, me->_jpg_y, jdec->width, jdec->height);

      me->_lcd->pushPixelsDMA(me->_dmabuf, jdec->width * h * 2);

    flip = !flip;
    me->_dmabuf = me->_dmabufs[flip];
    return 1;
  }
};

#endif
