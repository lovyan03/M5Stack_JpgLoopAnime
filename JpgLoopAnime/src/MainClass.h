#ifndef _MAINCLASS_H_
#define _MAINCLASS_H_

#pragma GCC optimize ("O3")

#include "tjpgdClass.h"
#include "DMADrawer.h"

class MainClass
{
public:
  bool setup(uint16_t width, uint16_t height, uint32_t spi_freq, int8_t tft_mosi, int8_t tft_miso, int8_t tft_sclk, int8_t tft_cs, int8_t tft_dc)
  {
    Serial.println("MainClass setup.");

    _tft_width = width;
    _tft_height = height;

    DMADrawer::setup(DMA_BUF_LEN, spi_freq, tft_mosi, tft_miso, tft_sclk, tft_cs, tft_dc);

   _jdec.multitask_begin();

  }

  bool drawJpg(uint8_t* buf, int32_t len) {
    _filebuf = buf;
    _fileindex = 0;
    _remain = len;
    JRESULT jres = _jdec.prepare(jpgRead, this);
    if (jres != JDR_OK) {
      Serial.printf("prepare failed! %d\r\n", jres);
      return false;
    }
    _jpg_x = (_tft_width - _jdec.width) / 2;
    _jpg_y = (_tft_height- _jdec.height) / 2;
    jres = _jdec.decomp_multitask(jpgWrite, jpgWriteRow, _lineskip);
    if (jres != JDR_OK) {
      Serial.printf("decomp failed! %d\r\n", jres);
      return false;
    }
    return true;
  }

private:
  enum
  { DMA_BUF_LEN = 320 * 16 * 2 // 320x16 16bit color
  , TCP_BUF_LEN = 512
  };

  TJpgD _jdec;
  int32_t _remain = 0;
  uint8_t* _filebuf;
  uint32_t _fileindex;

  uint32_t _sec = 0;
  uint16_t _tft_width;
  uint16_t _tft_height;
  uint16_t _jpg_x;
  uint16_t _jpg_y;
  uint16_t _drawCount = 0;
  uint16_t _delayCount = 0;
  uint8_t _lineskip = 0;
  uint8_t _tcpBuf[TCP_BUF_LEN];
  bool _recv_requested = false;

  static uint16_t jpgRead(TJpgD *jdec, uint8_t *buf, uint16_t len) {
    MainClass* me = (MainClass*)jdec->device;
    if (len > me->_remain)  len = me->_remain;
    if (buf) {
      memcpy(buf, (const uint8_t *)me->_filebuf + me->_fileindex, len);
    }
    me->_fileindex += len;
    me->_remain -= len;
    return len;
  }

  static uint16_t jpgWrite(TJpgD *jdec, void *bitmap, JRECT *rect) {
    MainClass* me = (MainClass*)jdec->device;
    uint16_t *p = DMADrawer::getNextBuffer();
    uint16_t *data = (uint16_t*)bitmap;
    uint16_t *dst;
    uint16_t width = jdec->width;
    uint16_t x = rect->left;
    uint16_t y = rect->top;
    uint8_t w = rect->right + 1 - x;
    uint8_t h = rect->bottom + 1 - y;
    uint8_t line;

    p += x + width * (y % ((1 + me->_lineskip) << 4));
    while (h--) {
      dst = p;
      line = w;
      while (line--) {
        *dst++ = *data++;
      }
      p += width;
    }

    return 1;
  }

  static uint16_t jpgWriteRow(TJpgD *jdec, uint16_t y, uint8_t h) {
    MainClass* me = (MainClass*)jdec->device;
    DMADrawer::draw( me->_jpg_x
                   , me->_jpg_y + y
                   , jdec->width
                   , h
                   );
    return 1;
  }
};

#endif
