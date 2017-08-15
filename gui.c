#include <SDL2/SDL.h>

#include "gui.h"

static SDL_Rect button_rect[NUM_BUTTONS] = {
	{ 565, 255, 70, 20 },
	{ 640, 255, 70, 20 },
	{ 565, 280, 145, 20 },
	{ 565, 305, 145, 20 },
	{ 565, 330, 70, 20 },
	{ 640, 330, 70, 20 },
	{ 565, 355, 145, 20 },
	{ 565, 380, 70, 20 },
	{ 640, 380, 70, 20 },
};

int get_button_at(int x, int y)
{
	for (int i=0; i<NUM_BUTTONS; i++) {
		SDL_Rect *r = &button_rect[i];
		int dx = x - r->x;
		if (dx < 0 || dx >= r->w) continue;
		int dy = y - r->y;
		if (dy < 0 || dy >= r->h) continue;
		return i;
	}
	return -1;
}
