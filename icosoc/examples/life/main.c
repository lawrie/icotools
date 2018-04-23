/*
 * The Game of Life
 *
 * Using 32x32 LED panel, picorv32 and icoboard.
 *
 * Original code: http://www.cs.utexas.edu/users/djimenez/utsa/cs1713-3/c/life.txt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"

/* dimensions of the screen */

#define BOARD_WIDTH	 31
#define BOARD_HEIGHT 31

/* set pixel function */

static inline void setled(int v)
{
	icosoc_leds(v);
}

void set_pixel(int x, int y, uint8_t c) //uint8_t r, uint8_t g, uint8_t b)
{
	icosoc_panel_setpixel(x,y,c ? 0xff : 0,0,0);
}

uint32_t i, j;

/* set everthing to random values */

void initialize(int board[][BOARD_HEIGHT])
{
	for (i = 0; i < BOARD_WIDTH; i++)
	for (j = 0; j < BOARD_HEIGHT; j++)
		board[i][j] = !(rand() % 9);
}

/* add to a width index, wrapping around like a cylinder */

int xadd(int i, int a)
{
	i += a;
	while (i < 0) i += BOARD_WIDTH;
	while (i >= BOARD_WIDTH) i -= BOARD_WIDTH;
	return i;
}

/* add to a height index, wrapping around */

int yadd(int i, int a)
{
	i += a;
	while (i < 0) i += BOARD_HEIGHT;
	while (i >= BOARD_HEIGHT) i -= BOARD_HEIGHT;
	return i;
}

/* return the number of on cells adjacent to the i,j cell */

int adjacent_to(int board[][BOARD_HEIGHT], int i, int j)
{
	int	k, l, count;

	count = 0;

	/* go around the cell */

	for (k = -1; k <= 1; k++) for (l = -1; l <= 1; l++)

		/* only count if at least one of k,l isn't zero */

		if (k || l)
			if (board[xadd(i,k)][yadd(j,l)]) count++;
	return count;
}

void play(int board[][BOARD_HEIGHT])
{
/*
	(copied this from some web page, hence the English spellings...)
	1.STASIS : If, for a given cell, the number of on neighbours is
	exactly two, the cell maintains its status quo into the next
	generation. If the cell is on, it stays on, if it is off, it stays off.
	2.GROWTH : If the number of on neighbours is exactly three, the cell
	will be on in the next generation. This is regardless of the cell's
	current state.
	3.DEATH : If the number of on neighbours is 0, 1, 4-8, the cell will
	be off in the next generation.
*/
	int	a, newboard[BOARD_WIDTH][BOARD_HEIGHT];

	/* for each cell, apply the rules of Life */

	for (i = 0; i < BOARD_WIDTH; i++)
	for (j = 0; j < BOARD_HEIGHT; j++)
	{
		a = adjacent_to(board, i, j);
		if (a == 2) newboard[i][j] = board[i][j];
		if (a == 3) newboard[i][j] = 1;
		if (a < 2) newboard[i][j] = 0;
		if (a > 3) newboard[i][j] = 0;
	}

	/* copy the new board back into the old board */

	for (i = 0; i < BOARD_WIDTH; i++)
	for (j = 0; j < BOARD_HEIGHT; j++)
		board[i][j] = newboard[i][j];
}

/* print the life board */

void print(int board[][BOARD_HEIGHT])
{
	for (i = 0; i < BOARD_WIDTH; i++)
	for (j = 0; j < BOARD_HEIGHT; j++)
    	set_pixel(i, j, board[i][j]);
}

/* main program */

void main()
{
	int board[BOARD_WIDTH][BOARD_HEIGHT];

	for (i = 0; i < 32; i++)
	for (j = 0; j < 32; j++)
		set_pixel(i, j, 0);

	initialize(board);
	print(board);

	while(1)
	{
		play(board);
		print(board);
		//for (int i = 0; i < 1000; i++)
		//	asm volatile ("");
	}
}
