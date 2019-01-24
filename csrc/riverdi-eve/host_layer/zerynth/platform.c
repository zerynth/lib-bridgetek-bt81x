/*
 * Copyright (c) Riverdi Sp. z o.o. sp. k. <riverdi@riverdi.com>
 * Copyright (c) Skalski Embedded Technologies <contact@lukasz-skalski.com>
 */

#include "platform.h"

static void
pabort (const char *msg)
{
  /* intentionally empty */
}


/*
 * platform_init()
 */
bool_t
platform_init (Gpu_HalInit_t *halinit)
{
  /* intentionally empty */
  return TRUE;
}


/*
 * platform_sleep_ms()
 */
void
platform_sleep_ms (uint32_t ms)
{
  vosThSleep(TIME_U(ms, MILLIS));
}


/*
 * platform_spi_init()
 */
bool_t
platform_spi_init (Gpu_Hal_Context_t *host)
{
  int ret;

  SpiPins *spipins = ((SpiPins*)_vm_pin_map(PRPH_SPI));

  vhalSpiConf bk_spi_conf;
  bk_spi_conf.clock    = SPI_SPEED_HZ;
  bk_spi_conf.mosi     = spipins[SPI_DEVICE].mosi;
  bk_spi_conf.miso     = spipins[SPI_DEVICE].miso;
  bk_spi_conf.sclk     = spipins[SPI_DEVICE].sclk;
  bk_spi_conf.nss      = GPIO_CS;
  bk_spi_conf.mode     = SPI_MODE;
  bk_spi_conf.bits     = SPI_BITS;
  bk_spi_conf.master   = 1;
  bk_spi_conf.msbfirst = 1;

  vhalSpiInit(SPI_DEVICE, &bk_spi_conf);

  // TODO: check SPI return value

  return TRUE;
}


/*
 * platform_spi_deinit()
 */
void
platform_spi_deinit (Gpu_Hal_Context_t *host)
{
  vhalSpiDone(SPI_DEVICE);
}


/*
 * platform_spi_send_recv_byte();
 */
uchar8_t
platform_spi_send_recv_byte (Gpu_Hal_Context_t  *host,
                             uchar8_t            data,
                             uint32_t            opt)
{
  int ret;
  // uint8_t recv;

  uint8_t to_recv[1];
  uint8_t to_send[1];

  to_send[0] = data;

  if (vhalSpiExchange(SPI_DEVICE, to_send, to_recv, 1)) {
    // error
  }
  return to_recv[0];
}


#define SPI_CHUNK 128

/*
 * platform_spi_send_data()
 */
uint16_t
platform_spi_send_data (Gpu_Hal_Context_t  *host,
                        uchar8_t           *data,
                        uint16_t            size,
                        uint32_t            opt)
{
  uint16_t curchunk, sent = 0;
  uint8_t pad_size = size % 4;
  uint8_t *pad_buf;

  while (sent < size) {
    curchunk = ((size - sent > SPI_CHUNK) ? SPI_CHUNK : size - sent);

    if (vhalSpiExchange(SPI_DEVICE, data + sent, NULL, curchunk)) {
      // error
    }

    sent += curchunk;
  }

  if ((opt == ZERYNTH_SPI_PAD) && pad_size) {
    pad_buf = gc_malloc(pad_size);
    vhalSpiExchange(SPI_DEVICE, pad_buf, NULL, pad_size);
    gc_free(pad_buf);
  }

  return size;
}


/*
 * platform_spi_recv_data()
 */
void
platform_spi_recv_data (Gpu_Hal_Context_t  *host,
                        uchar8_t           *data,
                        uint16_t            size,
                        uint32_t            opt)
{
  if (vhalSpiExchange(SPI_DEVICE, NULL, data, size)) {
    // error
  }
}

/*
 * platform_gpio_init()
 */
bool_t
platform_gpio_init (Gpu_Hal_Context_t *host,
                    gpio_name          ngpio)
{
  if (ngpio == GPIO_INT) {
    if (vhalPinSetMode(ngpio, PINMODE_INPUT_PULLUP)) {
      // return FALSE;
    }
    // vhalPinAttachInterrupt(ngpio, PINMODE_EXT_FALLING, interrupt_handler, 0);
    return TRUE;
  }
  // printf("gpio init %i\n", ngpio);
  if (vhalPinSetMode(ngpio, PINMODE_OUTPUT_PUSHPULL)) {
    // return FALSE;
  }
  return TRUE;
}


/*
 * platform_gpio_value()
 */
bool_t
platform_gpio_value (Gpu_Hal_Context_t  *host,
                     gpio_name           ngpio,
                     gpio_val            vgpio)
{
  // printf("gpio %i %i\n", ngpio, vgpio);
  if (vhalPinWrite(ngpio, vgpio)) {
    // return FALSE;
  }
  return TRUE;
}
