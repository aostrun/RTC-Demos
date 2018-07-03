#include <stdio.h>
#include "lift.h"
#include "lift_io.h"

#define HEIGHT  2+NUM_FLOOR*4+1
#define WIDTH  5+NUM_LIFT*8

static char door[4][4][6] =
{
	{ /* OPEN = 0 */
		"*****",
		"*   *",
		"*   *",
		"*****"
	},
	{ /* CLOSED = 1 */
		"*****",
		"* | *",
		"* | *",
		"*****"
	},
	{ /* OPENING = 2 */
		"*****",
		"*< >*",
		"*< >*",
		"*****"
	},
	{ /* CLOSING = 3 */
		"*****",
		"*> <*",
		"*> <*",
		"*****"
	}
};

void lift_print ( char *title, struct lift *lift, struct buttons *buttons )
{
	int i, j, k;
	char pic[HEIGHT][WIDTH+1];
	static unsigned int t_last_print = 0;
	unsigned int now;

	now = get_time();
	if ( t_last_print > now + 50 )
		return; /* don't print to frequently */
	t_last_print = now + 50;

	/* reset */
	for ( j = 0; j < HEIGHT; j++ ) {
		for ( i = 0; i < WIDTH; i++ )
			pic[j][i] = ' ';
		pic[j][i] = 0;
	}

	for ( j = 0; j < NUM_FLOOR; j++ ) {
		/* floor numbers */
		pic[ 3 + j*4 ][0] = '0' + NUM_FLOOR - j;

		/* floors */
		pic[ 2 + j*4 + 0 ][0] = '-';
		pic[ 2 + j*4 + 0 ][1] = '-';
		pic[ 2 + j*4 + 0 ][2] = '-';
		pic[ 2 + j*4 + 0 ][3] = '-';
		pic[ 2 + j*4 + 0 ][4] = '+';
		pic[ 2 + j*4 + 1 ][4] = '|';
		pic[ 2 + j*4 + 2 ][4] = '|';
		pic[ 2 + j*4 + 3 ][4] = '|';
		for ( i = 0; i < NUM_LIFT; i++ ) {
			for ( k = 1; k < 8; k++ )
				pic[ 2 + j*4 ][ 4 + i*8 + k ] = '-';
			pic[ 2 + j*4 + 0 ][ 4 + (i+1)*8 ] = '+';
			pic[ 2 + j*4 + 1 ][ 4 + (i+1)*8 ] = '|';
			pic[ 2 + j*4 + 2 ][ 4 + (i+1)*8 ] = '|';
			pic[ 2 + j*4 + 3 ][ 4 + (i+1)*8 ] = '|';
		}
	}

	/* last line */
	pic[ 2 + NUM_FLOOR*4 + 0 ][0] = '-';
	pic[ 2 + NUM_FLOOR*4 + 0 ][1] = '-';
	pic[ 2 + NUM_FLOOR*4 + 0 ][2] = '-';
	pic[ 2 + NUM_FLOOR*4 + 0 ][3] = '-';
	pic[ 2 + NUM_FLOOR*4 + 0 ][4] = '+';
	for ( i = 0; i < NUM_LIFT; i++ ) {
		for ( k = 1; k < 8; k++ )
			pic[ 2 + NUM_FLOOR*4 ][ 4 + i*8 + k ] = '-';
		pic[ 2 + NUM_FLOOR*4 ][ 4 + (i+1)*8 ] = '+';
	}

	for ( i = 0; i < NUM_LIFT; i++ ) {
		int offset_x, offset_y;
		char s[] = "-v^";
		pic[0][7+i*8] = s[lift[i].dir+1];
		pic[0][9+i*8] = '0' + ROUND(lift[i].floor);
		offset_x = 5 + ( 8 - NUM_FLOOR - 1 ) / 2;
		for ( j = 0; j < NUM_FLOOR; j++ )
			if ( buttons->in[i][j] )
				pic[1][offset_x+i*8+j] = '*';
			else
				pic[1][offset_x+i*8+j] = '-';

		offset_y = 3 + 4 * ( NUM_FLOOR - FLOOR (lift[i].floor) - 1 ) -
			ROUND ( ( lift[i].floor - FLOOR(lift[i].floor) ) * 4 );
		offset_x = 6 + i * 8;
		for ( j = 0; j < 4; j++ )
			for ( k = 0; k < 5; k++ )
				pic[offset_y+j][offset_x+k] = door[lift[i].door][j][k];
	}

	for ( j = 0; j < NUM_FLOOR; j++ ) {
		if ( buttons->out[j][0] )
			pic[ 5 + (NUM_FLOOR - j - 1) * 4 ][2] = 'v';
		else
			pic[ 5 + (NUM_FLOOR - j - 1) * 4 ][2] = ' ';
		if ( buttons->out[j][1] )
			pic[ 4 + (NUM_FLOOR - j - 1) * 4 ][2] = '^';
		else
			pic[ 4 + (NUM_FLOOR - j - 1) * 4 ][2] = ' ';
	}

	printf ( "\033[2J\033[1;1H" ); /* clear screen */
	for ( j = 0; j < WIDTH + 1; j++ )
		printf ( "#" );
	printf ( "\n%s\n", title );
	for ( j = 0; j < HEIGHT; j++ )
		printf ( "%s\n", pic[j] );
	printf ( "\n" );
	printf("0: %f,  1: %f \n\n", lift[0].floor, lift[1].floor);
}

#if 0
int main()
{
	struct lift lift[2] = {
		{
			.floor = 1,
			.dir = UP,
			.door = CLOSING,
			.state = STOPPED
		},
		{
			.floor = 3.7,
			.dir = DOWN,
			.door = CLOSED,
			.state = MOVING
		}
	};
	struct buttons buttons = {
		.in = {{1,0,1,0,1},{0,1,0,1,0}},
		.open = 0,
		.stop = 0,
		.cont = 0,
		.out = {{0,1},{0,1},{1,0},{1,1},{1,0}}
	};

	lift_print ( "LIFT MODEL", lift, &buttons );

	return 0;
}
#endif
