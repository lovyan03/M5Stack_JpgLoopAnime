#ifndef _MAINCLASS_H_
#define _MAINCLASS_H_

#pragma GCC optimize ("O3")

#include <esp_heap_caps.h>

#ifndef LOVYANGFX_HPP_
#include <LovyanGFX.hpp>  // https://github.com/lovyan03/LovyanGFX/
#endif

#include "tjpgdClass.h"

class MainClass
{
public:
  bool setup(LovyanGFX* lcd)
  {
    Serial.println("MainClass setup.");

    _lcd = lcd;
    _lcd_width = lcd->width();
    _lcd_height = lcd->height();
    _bytesize = lcd->getColorConverter()->bytes;
    _fp_jpgWrite = (_bytesize == 2)
                 ? jpgWrite16
                 : jpgWrite24
                 ;
    for (int i = 0; i < 2; ++i) _dmabufs[i] = (uint8_t*)heap_caps_malloc(_lcd_width * 48 * _bytesize, MALLOC_CAP_DMA);

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

    _out_width = std::min<int32_t>(_jdec.width, _lcd_width);
    _jpg_x = (_lcd_width - _jdec.width) >> 1;
    if (0 > _jpg_x) {
      _off_x = - _jpg_x;
      _jpg_x = 0;
    } else {
      _off_x = 0;
    }
    _out_height = std::min<int32_t>(_jdec.height, _lcd_height);
    _jpg_y = (_lcd_height- _jdec.height) >> 1;
    if (0 > _jpg_y) {
      _off_y = - _jpg_y;
      _jpg_y = 0;
    } else {
      _off_y = 0;
    }

    jres = _jdec.decomp_multitask(_fp_jpgWrite, jpgWriteRow);
    if (jres != TJpgD::JDR_OK) {
      Serial.printf("decomp failed! %d\r\n", jres);
      return false;
    }
    return true;
  }

private:
  LovyanGFX* _lcd;
  uint_fast8_t _bytesize;
  uint8_t* _dmabufs[2];
  uint8_t* _dmabuf;
  TJpgD _jdec;

  int32_t _remain = 0;
  const uint8_t* _filebuf;
  uint32_t _fileindex;

  int32_t _lcd_width;
  int32_t _lcd_height;
  int32_t _out_width;
  int32_t _out_height;
  int32_t _off_x;
  int32_t _off_y;
  int32_t _jpg_x;
  int32_t _jpg_y;

  uint32_t(*_fp_jpgWrite)(TJpgD*,void*,TJpgD::JRECT*);

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

  // for 24bit color panel
  static uint32_t jpgWrite24(TJpgD *jdec, void *bitmap, TJpgD::JRECT *rect) {
    MainClass* me = (MainClass*)jdec->device;

    auto dst = me->_dmabuf;

    uint_fast16_t x = rect->left;
    uint_fast16_t y = rect->top;
    uint_fast16_t w = rect->right + 1 - x;
    uint_fast16_t h = rect->bottom + 1 - y;
    uint_fast16_t outWidth = me->_out_width;
    uint_fast16_t outHeight = me->_out_height;
    uint8_t *src = (uint8_t*)bitmap;
    uint_fast16_t oL = 0, oR = 0;

    if (rect->right < me->_off_x)      return 1;
    if (x >= (me->_off_x + outWidth))  return 1;
    if (rect->bottom < me->_off_y)     return 1;
    if (y >= (me->_off_y + outHeight)) return 1;

    int32_t src_idx = 0;
    int32_t dst_idx = 0;

    if (me->_off_y > y) {
      uint_fast16_t linesToSkip = me->_off_y - y;
      src_idx += linesToSkip * w;
      h -= linesToSkip;
    }

    if (me->_off_x > x) {
      oL = me->_off_x - x;
    }
    if (rect->right >= (me->_off_x + outWidth)) {
      oR = (rect->right + 1) - (me->_off_x + outWidth);
    }

    int_fast16_t line = (w - ( oL + oR )) * 3;
    dst_idx += oL + x - me->_off_x;
    src_idx += oL;
    do {
      memcpy(&dst[dst_idx * 3], &src[src_idx * 3], line);
      dst_idx += outWidth;
      src_idx += w;
    } while (--h);

    return 1;
  }

  // for 16bit color panel
  static uint32_t jpgWrite16(TJpgD *jdec, void *bitmap, TJpgD::JRECT *rect) {
    MainClass* me = (MainClass*)jdec->device;

    uint16_t *dst = (uint16_t *)me->_dmabuf;

    uint_fast16_t x = rect->left;
    uint_fast16_t y = rect->top;
    uint_fast16_t w = rect->right + 1 - x;
    uint_fast16_t h = rect->bottom + 1 - y;
    uint_fast16_t outWidth = me->_out_width;
    uint_fast16_t outHeight = me->_out_height;
    uint8_t *src = (uint8_t*)bitmap;
    uint_fast16_t oL = 0, oR = 0;

    if (rect->right < me->_off_x)      return 1;
    if (x >= (me->_off_x + outWidth))  return 1;
    if (rect->bottom < me->_off_y)     return 1;
    if (y >= (me->_off_y + outHeight)) return 1;

    if (me->_off_y > y) {
      uint_fast16_t linesToSkip = me->_off_y - y;
      src += linesToSkip * w * 3;
      h -= linesToSkip;
    }

    if (me->_off_x > x) {
      oL = me->_off_x - x;
    }
    if (rect->right >= (me->_off_x + outWidth)) {
      oR = (rect->right + 1) - (me->_off_x + outWidth);
    }

    int_fast16_t line = (w - ( oL + oR ));
    dst += oL + x - me->_off_x;
    src += oL * 3;
    do {
      int i = 0;
      do {
        uint_fast8_t r8 = src[i*3+0] & 0xF8;
        uint_fast8_t g8 = src[i*3+1];
        uint_fast8_t b5 = src[i*3+2] >> 3;
        r8 |= g8 >> 5;
        g8 &= 0x1C;
        b5 = (g8 << 3) + b5;
        dst[i] = r8 | b5 << 8;
      } while (++i != line);
      dst += outWidth;
      src += w * 3;
    } while (--h);

    return 1;
  }

  static uint32_t jpgWriteRow(TJpgD *jdec, uint32_t y, uint32_t h) {
    static int flip = 0;
    MainClass* me = (MainClass*)jdec->device;
    if (y == 0) {
      me->_lcd->setAddrWindow(me->_jpg_x, me->_jpg_y, jdec->width, jdec->height);
    }
    me->_lcd->pushPixelsDMA(me->_dmabuf, jdec->width * h);

    flip = !flip;
    me->_dmabuf = me->_dmabufs[flip];
    return 1;
  }
};

#endif
