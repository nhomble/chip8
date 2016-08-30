#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"
#include "ui.h"

void destroy_window(WINDOW *win){
	wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(win);
	delwin(win);
	endwin();
}

WINDOW* create_win(int height, int width){
	WINDOW *local;
	local = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);
	box(local, 0, 0);
	wrefresh(local);
	return local;
}

static inline void handle_pixel(WINDOW *win, unsigned int x, unsigned int y){
	wmove(win, y, x);
	waddch(win, ACS_BLOCK);
}

// drawing with ncurses
void draw(WINDOW *win, struct chip8 *chip8){
	unsigned int x, y;
	wclear(win);	// clear ncurses screen
	for(y = 0; y < SCREEN_WIDTH; ++y)
		for(x = 0; x < SCREEN_LENGTH; ++x)
			if(chip8->gfx[x][y])
				handle_pixel(win, x, y);
	wrefresh(win);	// refresh screen to actually show the update
}

// ncurses things
void init_screen(){
	initscr();
	noecho();
	cbreak();
	curs_set(FALSE);
	timeout(KEY_TIMEOUT);
}


