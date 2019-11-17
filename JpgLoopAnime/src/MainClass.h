#ifndef _MAINCLASS_H_
#define _MAINCLASS_H_

#pragma GCC optimize ("O3")

#include "tjpgdClass.h"
#include "DMADrawer.h"

class MainClass
{
public:
  bool setup(TFT_eSPI* tft)
  {
    Serial.println("MainClass setup.");
    setup_t s;
    tft->getSetup(s);

    _tft_width = tft->width();
    _tft_height = tft->height();

    _dma.init(s);
    _dmabufs[0] = (uint16_t*)pvPortMallocCaps(_tft_width * 16 * 2, MALLOC_CAP_DMA);
    _dmabufs[1] = (uint16_t*)pvPortMallocCaps(_tft_width * 16 * 2, MALLOC_CAP_DMA);
    _dmabuf = _dmabufs[0];

   _jdec.multitask_begin();
  }

  bool drawJpg(const uint8_t* buf, int32_t len) {
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
    jres = _jdec.decomp_multitask(jpgWrite, jpgWriteRow);
    if (jres != JDR_OK) {
      Serial.printf("decomp failed! %d\r\n", jres);
      return false;
    }
    return true;
  }

  void wait() { delay(10); _dma.wait(); }

private:
  DMADrawer _dma;
  uint16_t* _dmabufs[2];
  uint16_t* _dmabuf;
  TJpgD _jdec;

  int32_t _remain = 0;
  const uint8_t* _filebuf;
  uint32_t _fileindex;

  uint16_t _tft_width;
  uint16_t _tft_height;
  uint16_t _jpg_x;
  uint16_t _jpg_y;

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
    uint16_t *p = me->_dmabuf;
    uint16_t *data = (uint16_t*)bitmap;
    uint16_t *dst;
    uint16_t width = jdec->width;
    uint16_t x = rect->left;
    uint16_t y = rect->top;
    uint8_t w = rect->right + 1 - x;
    uint8_t h = rect->bottom + 1 - y;
    uint8_t line;

    p += x;
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
    static int flip = 0;
    MainClass* me = (MainClass*)jdec->device;
    me->_dma.draw( me->_jpg_x
                 , me->_jpg_y + y
                 , jdec->width
                 , h
                 , me->_dmabuf
                 );
    flip = !flip;
    me->_dmabuf = me->_dmabufs[flip];
    return 1;
  }
};

#endif
