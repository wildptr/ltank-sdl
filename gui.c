#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "gui.h"

#define BLACK		  0,  0,  0,255
#define DARK_GRAY	128,128,128,255
#define LIGHT_GRAY	192,192,192,255
#define WHITE		255,255,255,255

extern TTF_Font *font;

struct container root;

struct widget_class container_class;
struct widget_class panel_class;
struct widget_class button_class;
struct widget_class image_widget_class;
struct widget_class canvas_class;
struct widget_class text_widget_class;

void widget_init(struct widget *w, struct widget_class *class)
{
	w->w = 0;
	w->h = 0;
	w->class = class;
}

void null_handler() {}

/*
 * Text area
 */

void text_area_paint(struct text_area *ta, int x, int y, int w, int h)
{
	SDL_Texture *tex = ta->tex;
	if (tex == NULL) return;
	int font_height = TTF_FontHeight(font);
	SDL_Rect dst;
	dst.x = x + (w-ta->tex_w)/2;
	dst.y = y + (h-font_height)/2;
	dst.w = ta->tex_w;
	dst.h = ta->tex_h;
	SDL_RenderCopy(renderer, tex, NULL, &dst);
}

void text_area_init(struct text_area *ta)
{
	ta->text = NULL;
	ta->color.r = 255;
	ta->color.g = 255;
	ta->color.b = 255;
	ta->color.a = 255;
	ta->style = 0;
	ta->tex = NULL;
}

void text_area_set_text(struct text_area *ta, const char *text)
{
	if (ta->tex) {
		SDL_DestroyTexture(ta->tex);
	}
	ta->text = text;
	if (text != NULL) {
		SDL_Surface *s = TTF_RenderText_Blended(font, ta->text, ta->color);
		ta->tex = SDL_CreateTextureFromSurface(renderer, s);
		ta->tex_w = s->w;
		ta->tex_h = s->h;
		SDL_FreeSurface(s);
	} else {
		ta->tex = NULL;
		ta->tex_w = 0;
		ta->tex_h = 0;
	}
}

/*
 * Container class
 */

void container_button_down(struct container *w, int button, int x, int y)
{
	/*
	 * Iterate over children in reverse order since last child is on top.
	 */
	for (int i=w->n_child-1; i>=0; i--) {
		struct child *c = &w->children[i];
		struct widget *cw = c->w;
		int dx = x - c->x;
		if (dx < 0 || dx >= cw->w) continue;
		int dy = y - c->y;
		if (dy < 0 || dy >= cw->h) continue;
		cw->class->button_down(cw, button, dx, dy);
		break;
	}
}

void container_paint(struct container *w, int x, int y)
{
	for (int i=0; i<w->n_child; i++) {
		struct child *c = &w->children[i];
		struct widget *cw = c->w;
		cw->class->paint(cw, x + c->x, y + c->y);
	}
}

void container_init(struct container *w, int cap)
{
	widget_init(WIDGET(w), &container_class);
	w->children = malloc(cap * sizeof *w->children);
	w->n_child = 0;
}

/*
 * Panel class
 */

void panel_paint(struct panel *w, int x, int y)
{
	SDL_Rect dst = { x, y, w->w, w->h };
	SDL_SetRenderDrawColor(renderer, LIGHT_GRAY);
	SDL_RenderFillRect(renderer, &dst);

	switch (w->bevel) {
	case BEVEL_NONE:
		break;
	case BEVEL_INNER:
		draw_inner_bevel(&dst);
		break;
	case BEVEL_OUTER:
		draw_outer_bevel(&dst);
		break;
	}

	container_paint(CONTAINER(w), x, y);
}

void panel_init(struct panel *w, int cap, int bevel)
{
	container_init(CONTAINER(w), cap);
	w->class = &panel_class;
	w->bevel = bevel;
}

/*
 * Button class
 */

void button_button_down(struct button *w, int button, int x, int y)
{
	w->button_down(WIDGET(w), button, x, y);
}

void button_paint(struct button *w, int x, int y)
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w->w;
	int y1 = y + w->h;

	SDL_Rect rect[2];

	rect[0].x = x0;
	rect[0].y = y0;
	rect[0].w = w->w-1;
	rect[0].h = 1;
	rect[1].x = x0;
	rect[1].y = y0+1;
	rect[1].w = 1;
	rect[1].h = w->h-2;

	SDL_SetRenderDrawColor(renderer, WHITE);
	SDL_RenderFillRects(renderer, rect, 2);

	rect[0].x = x0+1;
	rect[0].y = y0+1;
	rect[0].w = w->w-3;
	rect[0].h = w->h-3;

	SDL_SetRenderDrawColor(renderer, LIGHT_GRAY);
	SDL_RenderFillRect(renderer, rect);

	rect[0].x = x0+1;
	rect[0].y = y1-2;
	rect[0].w = w->w-2;
	rect[0].h = 1;
	rect[1].x = x1-2;
	rect[1].y = y0+1;
	rect[1].w = 1;
	rect[1].h = w->h-2;

	SDL_SetRenderDrawColor(renderer, DARK_GRAY);
	SDL_RenderFillRects(renderer, rect, 2);

	rect[0].x = x0;
	rect[0].y = y1-1;
	rect[0].w = w->w;
	rect[0].h = 1;
	rect[1].x = x1-1;
	rect[1].y = y0;
	rect[1].w = 1;
	rect[1].h = w->h;

	SDL_SetRenderDrawColor(renderer, BLACK);
	SDL_RenderFillRects(renderer, rect, 2);

	text_area_paint(&w->caption, x, y, w->w, w->h);
}

void button_init(struct button *w, button_down_handler cb)
{
	widget_init(WIDGET(w), &button_class);
	w->button_down = cb;
}

/*
 * Image widget class
 */

void image_widget_paint(struct image_widget *w, int x, int y)
{
	SDL_Rect dst = { x, y, w->w, w->h };
	SDL_RenderCopy(renderer, w->tex, NULL, &dst);
}

void image_widget_init(struct image_widget *w, SDL_Texture *tex)
{
	widget_init(WIDGET(w), &image_widget_class);
	w->tex = tex;
}

/*
 * Canvas class
 */

void canvas_button_down(struct canvas *w, int button, int x, int y)
{
	w->button_down(WIDGET(w), button, x, y);
}

void canvas_paint(struct canvas *w, int x, int y)
{
	w->paint(WIDGET(w), x, y);
}

void canvas_init(struct canvas *w)
{
	widget_init(WIDGET(w), &canvas_class);
}

/*
 * Text widget class
 */

void text_widget_paint(struct text_widget *w, int x, int y)
{
	text_area_paint(&w->text_area, x, y, w->w, w->h);
}

void text_widget_init(struct text_widget *w)
{
	widget_init(WIDGET(w), &text_widget_class);
	text_area_init(&w->text_area);
}

void init_gui(int root_cap)
{
	/*
	 * Initialize widget classes
	 */

	container_class.button_down = (button_down_handler) container_button_down;
	container_class.paint = (paint_handler) container_paint;

	panel_class.button_down = (button_down_handler) container_button_down;
	panel_class.paint = (paint_handler) panel_paint;

	button_class.button_down = (button_down_handler) button_button_down;
	button_class.paint = (paint_handler) button_paint;

	image_widget_class.button_down = null_handler;
	image_widget_class.paint = (paint_handler) image_widget_paint;

	canvas_class.button_down = (button_down_handler) canvas_button_down;
	canvas_class.paint = (paint_handler) canvas_paint;

	text_widget_class.button_down = null_handler;
	text_widget_class.paint = (paint_handler) text_widget_paint;

	/*
	 * Initialize root widget
	 */
	container_init(&root, root_cap);
};

void draw_gui(void)
{
	container_paint(&root, 0, 0);
}

void add_child(struct container *parent, struct widget *child, int x, int y)
{
	struct child *c = &parent->children[parent->n_child++];
	c->w = child;
	c->x = x;
	c->y = y;
}

void handle_button_down(SDL_MouseButtonEvent *e)
{
	container_button_down(&root, e->button, e->x, e->y);
}

void draw_inner_bevel(SDL_Rect *r)
{
	SDL_Rect rr[2];

	rr[0].x = r->x;
	rr[0].y = r->y;
	rr[0].w = r->w;
	rr[0].h = 1;
	rr[1].x = r->x;
	rr[1].y = r->y+1;
	rr[1].w = 1;
	rr[1].h = r->h-1;

	SDL_SetRenderDrawColor(renderer, DARK_GRAY);
	SDL_RenderFillRects(renderer, rr, 2);

	rr[0].x = r->x+1;
	rr[0].y = r->y+r->h-1;
	rr[0].w = r->w-1;
	rr[0].h = 1;
	rr[1].x = r->x+r->w-1;
	rr[1].y = r->y+1;
	rr[1].w = 1;
	rr[1].h = r->h-1;

	SDL_SetRenderDrawColor(renderer, WHITE);
	SDL_RenderFillRects(renderer, rr, 2);
}

void draw_outer_bevel(SDL_Rect *r)
{
	SDL_Rect rr[2];

	rr[0].x = r->x;
	rr[0].y = r->y;
	rr[0].w = r->w-1;
	rr[0].h = 1;
	rr[1].x = r->x;
	rr[1].y = r->y+1;
	rr[1].w = 1;
	rr[1].h = r->h-2;

	SDL_SetRenderDrawColor(renderer, WHITE);
	SDL_RenderFillRects(renderer, rr, 2);

	rr[0].x = r->x;
	rr[0].y = r->y+r->h-1;
	rr[0].w = r->w;
	rr[0].h = 1;
	rr[1].x = r->x+r->w-1;
	rr[1].y = r->y;
	rr[1].w = 1;
	rr[1].h = r->h;

	SDL_SetRenderDrawColor(renderer, DARK_GRAY);
	SDL_RenderFillRects(renderer, rr, 2);
}
