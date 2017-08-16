#include <SDL2/SDL.h>

#include "gui.h"
#include "palette.h"

void palette_button_down(struct palette *w, int button, int x, int y)
{
	int cell_x = x/36;
	int cell_y = y/36;

	int sel = cell_y * w->n_col + cell_x;

	switch (button) {
	case SDL_BUTTON_LEFT:
		w->selection1 = sel;
		break;
	case SDL_BUTTON_RIGHT:
		w->selection2 = sel;
		break;
	}
}

void palette_paint(struct palette *w, int x, int y)
{
	SDL_Rect dst, bevel;
	dst.w = 32;
	dst.h = 32;
	bevel.w = 34;
	bevel.h = 34;
	int i = 0;

	for (int cell_y = 0; cell_y < w->n_row; cell_y++) {
		for (int cell_x = 0; cell_x < w->n_col; cell_x++) {
			if (i >= w->n_sprite) break;
			bevel.x = x + 36*cell_x;
			bevel.y = y + 36*cell_y;
			dst.x = bevel.x + 1;
			dst.y = bevel.y + 1;
			SDL_RenderCopy(renderer, w->sprites[i], NULL, &dst);
			draw_outer_bevel(&bevel);
			i++;
		}
	}
}

struct widget_class palette_class = {
	(button_down_handler) palette_button_down,
	(paint_handler) palette_paint,
};

void palette_init(struct palette *w, int n_col, int n_sprite, SDL_Texture **sprites)
{
	widget_init(WIDGET(w), &palette_class);
	int n_row = (n_sprite+n_col-1)/n_col;
	w->w = 36*n_col-4;
	w->h = 36*n_row-4;
	w->n_col = n_col;
	w->n_row = n_row;
	w->n_sprite = n_sprite;
	w->sprites = sprites;
	w->selection1 = 0;
	w->selection2 = 0;
}
