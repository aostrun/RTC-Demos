/*
 * Initialization for raspberry pi IO, with example
 * compile with flag -lbcm2835 and run as superuser
 */
#ifdef USE_RPI

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "rpi_io.h"

void rpi_init ( size_t leds, int *led, size_t sensors, int *sensor )
{
	int i;

	if ( bcm2835_init() == 0 ) {
		perror ( "bcm2835_init" );
		exit(1);
	}

	for ( i = 0; i < leds; i++ ) {
		bcm2835_gpio_fsel ( led[i], BCM2835_GPIO_FSEL_OUTP );
		bcm2835_gpio_write ( led[i], LOW );
	}

	for ( i = 0; i < sensors; i++ ) {
		bcm2835_gpio_fsel ( sensor[i], BCM2835_GPIO_FSEL_OUTP );
		bcm2835_gpio_write ( sensor[i], HIGH );
		bcm2835_delay (10);
		bcm2835_gpio_fsel ( sensor[i], BCM2835_GPIO_FSEL_INPT );
		bcm2835_gpio_set_pud ( sensor[i], BCM2835_GPIO_PUD_UP );
	}
}

#if 0
/*
 * just system test -- with lift
 * on raspberry pi compile with:
 *    gcc rpi_io.c -Wall -lbcm2835 -lrt
 * and run with:
 *    sudo ./a.out
 */

int led[] = {28,30,2,3,4, 14,15, 8,11,25,9,29, 24,10};
int sensor[] = {18,17,27,22,23,31,7};

#define SIZEOF(X)   (sizeof(X)/sizeof(X[0]))

int main() {
	int i, j;

	rpi_init ( SIZEOF(led), led, SIZEOF(sensor), sensor );

	/* blinking 10 seconds */
	for ( j = 0; j < 50; j++ ) {
		for ( i = 0; i < SIZEOF(led); i++ ) {
			if ( (j & 1) && j < 49 )
				rpi_output ( led[i], ON );
			else
				rpi_output ( led[i], OFF );
		}
		bcm2835_delay(200);
	}

	/* light floor led in lift based on sensor */
	for ( j = 0; j < 100; j++ ) {
		for ( i = 0; i < SIZEOF(sensor); i++ ) {
			rpi_output ( led[i], rpi_input(sensor[i]) );
			rpi_output ( led[i+7], rpi_input(sensor[i]) );
		}
		bcm2835_delay(100);
	}

	rpi_close();

	return 0;
}
#endif

#endif
