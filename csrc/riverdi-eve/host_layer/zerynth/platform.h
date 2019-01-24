/*
 * Copyright (c) Riverdi Sp. z o.o. sp. k. <riverdi@riverdi.com>
 * Copyright (c) Skalski Embedded Technologies <contact@lukasz-skalski.com>
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

/* C library inclusions */
#define ZERYNTH_PRINTF
#include "zerynth.h"

#define ZERYNTH_PLATFORM
#define ZERYNTH_WAITFIFO_TIMEOUT 20000

typedef enum {
    ZERYNTH_WAITFIFO_ERROR_TIMEOUT = 1,
    ZERYNTH_WAITFIFO_ERROR_FAULT,
} zerynth_waitfifo_error_t;

extern zerynth_waitfifo_error_t zerynth_waitfifo_error;
extern uint32_t zerynth_waitfifo_timeout;

#define ZERYNTH_SPI_PAD 0xaa

/*****************************************************************************/

/* type definitions for EVE HAL library */

#define TRUE		(1)
#define FALSE		(0)

typedef char		bool_t;
typedef char		char8_t;
typedef unsigned char	uchar8_t;
typedef signed char	schar8_t;
typedef float		float_t;

/* Predefined Riverdi modules */
//#include "modules.h"
#if defined (EVE_1)
  #define FT80X_ENABLE
#elif defined (EVE_2)
  #define FT81X_ENABLE
#elif defined (EVE_3)
  #define BT81X_ENABLE
  #define FT81X_ENABLE
#else
  #error "Please choose generation of Embedded Video Engine (EVE_1, EVE_2, EVE_3)"
#endif

/* EVE inclusions */
#include "Gpu_Hal.h"
#include "Gpu.h"
#include "CoPro_Cmds.h"
#include "Hal_Utils.h"

/*****************************************************************************/

// #define SPI_DEVICE	0
extern int SPI_DEVICE;
extern int SPI_SPEED_HZ;

#define SPI_MODE	SPI_MODE_LOW_FIRST
#define SPI_BITS	SPI_BITS_8

typedef int gpio_name;
extern gpio_name GPIO_CS;
extern gpio_name GPIO_PD;
extern gpio_name GPIO_INT;

// typedef enum {
//   GPIO_CS   = D27,
//   GPIO_PD   = D25,
//   GPIO_INT  = D26
// } gpio_name;

typedef enum {
  GPIO_HIGH = 1,
  GPIO_LOW  = 0
} gpio_val;

/*****************************************************************************/

bool_t platform_init (Gpu_HalInit_t*);
void platform_sleep_ms (uint32_t);

bool_t platform_spi_init (Gpu_Hal_Context_t*);
void platform_spi_deinit (Gpu_Hal_Context_t*);

uchar8_t platform_spi_send_recv_byte (Gpu_Hal_Context_t*, uchar8_t, uint32_t);
uint16_t platform_spi_send_data (Gpu_Hal_Context_t*, uchar8_t*, uint16_t, uint32_t);
void platform_spi_recv_data (Gpu_Hal_Context_t*, uchar8_t*, uint16_t, uint32_t);

bool_t platform_gpio_init (Gpu_Hal_Context_t*, gpio_name);
bool_t platform_gpio_value (Gpu_Hal_Context_t*, gpio_name, gpio_val);

/*****************************************************************************/

#endif /*_PLATFORM_H_*/
