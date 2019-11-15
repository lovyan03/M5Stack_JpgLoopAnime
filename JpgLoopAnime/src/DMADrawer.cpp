#pragma GCC optimize ("O3")

#include <esp_heap_alloc_caps.h>

#include "DMADrawer.h"

void DMADrawer::lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
  gpio_set_level(((DMADrawer*)t->user)->_tft_dc, (t->length != 8));
  gpio_set_level(((DMADrawer*)t->user)->_tft_cs, 0);
}

void DMADrawer::lcd_spi_post_transfer_callback(spi_transaction_t *t)
{
  gpio_set_level(((DMADrawer*)t->user)->_tft_cs, 1);
}

void DMADrawer::init(const setup_t& s)
{
  _last_x = -1;
  _last_y = -1;
  _last_w = -1;
  _last_h = -1;
  for (int i = 0; i < 6; ++i) {
    _sent[i] = false;
    _trans[i].user = this;
    _trans[i].flags = SPI_TRANS_USE_TXDATA;
    _trans[i].cmd = _trans[i].addr = _trans[i].rxlength = 0;
    _trans[i].length = (i & 1) ? 8 * 4 : 8;
  }
  _trans[0].tx_data[0] = 0x2A; // Column address set
  _trans[2].tx_data[0] = 0x2B; // Page address set
  _trans[4].tx_data[0] = 0x2C; // Memory write
  _trans[5].flags = 0;

  _tft_dc = (gpio_num_t)s.pin_tft_dc;
  _tft_cs = (gpio_num_t)s.pin_tft_cs;
  _offset_x = s.r0_x_offset;
  _offset_y = s.r0_y_offset;

  if (_spi_handle == NULL)  {
    spi_bus_config_t buscfg = {
        .mosi_io_num = (gpio_num_t)s.pin_tft_mosi,
        .miso_io_num = (gpio_num_t)s.pin_tft_miso,
        .sclk_io_num = (gpio_num_t)s.pin_tft_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = s.tft_width * s.tft_height * 2 + 8,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .intr_flags = 0};
    ESP_ERROR_CHECK(spi_bus_initialize((s.port == VSPI) ? VSPI_HOST : HSPI_HOST, &buscfg, 1));

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0, //SPI mode 0
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = s.tft_spi_freq * 100000,
        .input_delay_ns = 0,
        .spics_io_num = (gpio_num_t)s.pin_tft_cs,
        .flags = SPI_DEVICE_NO_DUMMY,
        .queue_size = 6,
        .pre_cb = lcd_spi_pre_transfer_callback,
        .post_cb = lcd_spi_post_transfer_callback};
    ESP_ERROR_CHECK(spi_bus_add_device((s.port == VSPI) ? VSPI_HOST : HSPI_HOST, &devcfg, &_spi_handle));
  }
}

void DMADrawer::draw(uint16_t x, uint16_t y, TFT_eSprite* sprite)
{
  draw(x, y, sprite->width(), sprite->height(), sprite->frameBuffer(0));
}

void DMADrawer::draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, void* data)
{
  esp_err_t ret;
  spi_transaction_t *tr;

  x += _offset_x;
  y += _offset_y;
  for (uint8_t i = 0; i < 6; i++)
  {
    if (_sent[i]) {
      _sent[i] = false;
      ret = spi_device_get_trans_result(_spi_handle, &tr, portMAX_DELAY);
      assert(ret == ESP_OK);
    }
    tr = &_trans[i];
    switch (i) {
    case 0:
      if (_last_x == x && _last_w == w) continue;
      break;

    case 1:
      if (_last_x == x && _last_w == w) continue;
      tr->tx_data[0] = x >> 8;             //Start Col High
      tr->tx_data[1] = x & 0xFF;           //Start Col Low
      tr->tx_data[2] = (x + w - 1) >> 8;   //End Col High
      tr->tx_data[3] = (x + w - 1) & 0xFF; //End Col Low
      break;

    case 2:
      if (_last_y == y && _last_h == h) continue;
      break;

    case 3:
      if (_last_y == y && _last_h == h) continue;
      tr->tx_data[0] = y >> 8;             //Start page high
      tr->tx_data[1] = y & 0xFF;           //start page low
      tr->tx_data[2] = (y + h - 1) >> 8;   //end page high
      tr->tx_data[3] = (y + h - 1) & 0xFF; //end page low
      break;

    case 5:
      tr->tx_buffer = data;               //finally send the line data
      tr->length = w * h * 16;            //Data length, in bits
      break;

    default:
      break;
    }
    tr->rxlength = 0;
    ret = spi_device_queue_trans(_spi_handle, tr, portMAX_DELAY);
    assert(ret == ESP_OK);
    _sent[i] = true;
  }
  _last_x = x;
  _last_y = y;
  _last_w = w;
  _last_h = h;
}

void DMADrawer::wait()
{
  spi_transaction_t *rtrans;
  esp_err_t ret;

  _last_x = -1;
  _last_y = -1;
  _last_w = -1;
  _last_h = -1;

  for (uint8_t i = 0; i < 6; ++i)
  {
    if (!_sent[i]) continue;
    _sent[i] = false;
    ret = spi_device_get_trans_result(_spi_handle, &rtrans, portMAX_DELAY);
    assert(ret == ESP_OK);
  }
}

//------------------------------------------------------------------

void* DMA_eSprite::createDMA(int16_t w, int16_t h)
{
  setColorDepth(16);

  _iwidth  = _dwidth  = _bitwidth = w;
  _iheight = _dheight = h;

  cursor_x = cursor_y = 0;

  _sx = _sy = 0;
  _sw = w;
  _sh = h;
  _scolor = TFT_BLACK;

  _xpivot = w/2;
  _ypivot = h/2;

  _img8   = (uint8_t*)pvPortMallocCaps(w * h * 2, MALLOC_CAP_DMA);
  _img8_1 = _img8_2 = _img8;
  _img    = (uint16_t*) _img8;

  if (_img8)
  {
    _created = true;
    return _img8;
  }

  return NULL;
}

