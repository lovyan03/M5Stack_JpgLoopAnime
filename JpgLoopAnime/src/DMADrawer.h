#ifndef _DMADRAWER_H_
#define _DMADRAWER_H_

#include <driver/spi_master.h>

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
  #include <M5Stack.h>
#elif defined(ARDUINO_M5Stick_C)
  #include <M5StickC.h>
#else
  #include <TFT_eSPI.h>  // https://github.com/Bodmer/TFT_eSPI
#endif

//------------------------------------------------------------------

class DMADrawer
{
public:
  void init(const setup_t& s);
  void draw(uint16_t x, uint16_t y, TFT_eSprite* sprite);
  void draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, void* data);
  void wait();

private:
  spi_device_handle_t _spi_handle = NULL;
  uint16_t _last_x, _last_y, _last_w, _last_h;
  uint16_t _offset_x, _offset_y;
  spi_transaction_t _trans[6];
  bool _sent[6];
  gpio_num_t _tft_dc, _tft_cs;

  static void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
  static void lcd_spi_post_transfer_callback(spi_transaction_t *t);
};

//------------------------------------------------------------------

class DMA_eSprite : public TFT_eSprite
{
public:
  DMA_eSprite(TFT_eSPI* tft = NULL) : TFT_eSprite(tft) {}
  void* createDMA(int16_t w, int16_t h);
};

//------------------------------------------------------------------

#endif
