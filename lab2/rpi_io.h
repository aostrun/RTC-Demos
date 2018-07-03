#pragma once

#ifdef USE_RPI

#include <stdlib.h>
#include <bcm2835.h>

#define ON  1
#define OFF 0

#define OUT_LEVEL(X)		( (X) >= ON ? HIGH : LOW )
#define IN_LEVEL(X)		( (X) ? OFF : ON ) /* active in low state */

#define rpi_input(pin)		IN_LEVEL ( bcm2835_gpio_lev (pin) )
#define rpi_output(pin,state)	bcm2835_gpio_write ( pin, OUT_LEVEL(state) )
#define rpi_close()		bcm2835_close ()
void rpi_init ( size_t leds, int *led, size_t sensors, int *sensor);

#endif
