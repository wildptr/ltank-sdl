/*
 *      main.c
 *
 *      Copyright 2009 Chaitanya Talnikar<chaitukca@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 3 of the License.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *       along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "gui.h"
#include "level.h"
#include "palette.h"

// pixel offset of game board from top-left corner of window
#define BOARD_X 17
#define BOARD_Y 17

#define WINDOW_W 734
#define WINDOW_H 547

#define NUM_PALETTE_SPRITES 26

// prints error message if fails
SDL_Surface *util_LoadBMP(const char *path);

int load_graphic_set(const char *path);

SDL_Surface *screen;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Surface *sprites[60];

int num_levels;
struct level *levels;
int current_level;

bool anim_on;
int anim_delay;
int anim_phase;

struct panel *game_panel;
struct panel *control_panel;
struct palette *editor_palette;

bool editor_on;

void render(void)
{
	SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void open_editor(void)
{
	game_panel->n_child--;
	add_child(CONTAINER(game_panel), WIDGET(editor_palette), 4, 14);
	editor_on = true;
}

void close_editor(void)
{
	game_panel->n_child--;
	add_child(CONTAINER(game_panel), WIDGET(control_panel), 10, 5);
	editor_on = false;
}

void start_level(void)
{
	struct level *l = &levels[current_level];
	for (int y=0; y<16; y++) {
		for (int x=0; x<16; x++) {
			uint8_t b = l->board[x][y];
			uint8_t fg = 0, bg = 0;
			switch (b) {
			case 0: break;
			case 1:
				tank_y = y;
				tank_x = x;
				break;
			case 2: bg = B_FLAG; break;
			case 3: bg = B_WATER; break;
			case 4: fg = F_WALL; break;
			case 5: fg = F_CRATE; break;
			case 6: fg = F_BRICK; break;
			case 7: fg = F_ANTI_N; break;
			case 8: fg = F_ANTI_E; break;
			case 9: fg = F_ANTI_S; break;
			case 10: fg = F_ANTI_W; break;
			case 11: fg = F_MIRROR_NW; break;
			case 12: fg = F_MIRROR_NE; break;
			case 13: fg = F_MIRROR_SE; break;
			case 14: fg = F_MIRROR_SW; break;
			case 15: bg = B_BELT_N; break;
			case 16: bg = B_BELT_E; break;
			case 17: bg = B_BELT_S; break;
			case 18: bg = B_BELT_W; break;
			case 19: fg = F_CRYSTAL; break;
			case 20: fg = F_ROT_MIRROR_NW; break;
			case 21: fg = F_ROT_MIRROR_NE; break;
			case 22: fg = F_ROT_MIRROR_SE; break;
			case 23: fg = F_ROT_MIRROR_SW; break;
			case 24: bg = B_ICE; break;
			case 25: bg = B_THIN_ICE; break;
			default: fg = F_WALL;
			}
			board[y][x].fg = fg;
			board[y][x].bg = bg;
		}
	}
	tank_orient = NORTH;
	tank_action = 0;
	tank_alive = true;
	num_lasers = 0;
	active = true;
	if (editor_on) {
		close_editor();
	}
}

int get_sprite_id(uint8_t obj, int anim_phase)
{
	int p = anim_phase;
	switch (obj) {
	case B_NOTHING:
		return 0;
	case B_FLAG:
		return 5+p;
	case B_WATER:
		return 8+p;
	case B_BRIDGE:
		return 18;
	case B_BELT_N:
		return 23+p;
	case B_BELT_E:
		return 26+p;
	case B_BELT_S:
		return 29+p;
	case B_BELT_W:
		return 32+p;
	case B_ICE:
		return 55;
	case B_THIN_ICE:
		return 56;
	case F_WALL:
		return 12;
	case F_CRATE:
		return 13;
	case F_BRICK:
		return 14;
	case F_CRYSTAL:
		return 44;
	case F_ANTI_N:
		return 15+p;
	case F_ANTI_E:
		return 35+p;
	case F_ANTI_S:
		return 38+p;
	case F_ANTI_W:
		return 41+p;
	case F_BLOWN_ANTI_N:
		return 53;
	case F_BLOWN_ANTI_E:
		return 51;
	case F_BLOWN_ANTI_S:
		return 11;
	case F_BLOWN_ANTI_W:
		return 52;
	case F_MIRROR_NE:
		return 20;
	case F_MIRROR_SE:
		return 21;
	case F_MIRROR_SW:
		return 22;
	case F_MIRROR_NW:
		return 19;
	case F_ROT_MIRROR_NE:
		return 47;
	case F_ROT_MIRROR_SE:
		return 48;
	case F_ROT_MIRROR_SW:
		return 49;
	case F_ROT_MIRROR_NW:
		return 46;
	}
	return 0;
}

void draw_tile(int y, int x, int anim_phase)
{
	struct tile *t = &board[y][x];
	SDL_Surface *bg, *fg;

	SDL_Rect dst;
	dst.x = BOARD_X + x*32;
	dst.y = BOARD_Y + y*32;
	dst.w = 32;
	dst.h = 32;

	bg = sprites[get_sprite_id(t->bg, anim_phase)];
	SDL_BlitSurface(bg, NULL, screen, &dst);
	if (t->fg) {
		fg = sprites[get_sprite_id(t->fg, anim_phase)];
		SDL_BlitSurface(fg, NULL, screen, &dst);
	}
}

void draw_tank(void)
{
	int sprite_id = 1 + tank_orient;
	SDL_Rect dst;

	dst.x = BOARD_X + tank_x*32;
	dst.y = BOARD_Y + tank_y*32;
	dst.w = 32;
	dst.h = 32;

	SDL_BlitSurface(sprites[sprite_id], NULL, screen, &dst);
}

void draw_laser(const struct laser *l)
{
	int y = l->y;
	int x = l->x;

	int color = l->style >> 4;
	int shape = l->style & 15;

	uint32_t sdl_color = SDL_MapRGB(screen->format,
					((color>>0)&1)*255,
					((color>>1)&1)*255,
					((color>>2)&1)*255);

	SDL_Rect dst, dst1, dst2;
	dst.x = BOARD_X + x*32;
	dst.y = BOARD_Y + y*32;

	switch (shape) {
	case 0:
		// N-S
		dst.x += 15;
		dst.w = 2;
		dst.h = 32;
		break;
	case 1:
		// E-W
		dst.y += 15;
		dst.w = 32;
		dst.h = 2;
		break;
	case 2:
		dst1.x = dst.x + 15;
		dst1.y = dst.y;
		dst1.w = 2;
		dst1.h = 16;
		dst2.x = dst.x + 16;
		dst2.y = dst.y + 15;
		dst2.w = 16;
		dst2.h = 2;
		goto l;
	case 3:
		dst1.x = dst.x + 16;
		dst1.y = dst.y + 15;
		dst1.w = 16;
		dst1.h = 2;
		dst2.x = dst.x + 15;
		dst2.y = dst.y + 16;
		dst2.w = 2;
		dst2.h = 16;
		goto l;
	case 4:
		dst1.x = dst.x + 15;
		dst1.y = dst.y + 16;
		dst1.w = 2;
		dst1.h = 16;
		dst2.x = dst.x;
		dst2.y = dst.y + 15;
		dst2.w = 16;
		dst2.h = 2;
		goto l;
	case 5:
		dst1.x = dst.x;
		dst1.y = dst.y + 15;
		dst1.w = 16;
		dst1.h = 2;
		dst2.x = dst.x + 15;
		dst2.y = dst.y;
		dst2.w = 2;
		dst2.h = 16;
		goto l;
	default:
		dst.x += 15;
		dst.y += 15;
		dst.w = 2;
		dst.h = 2;
	}

	SDL_FillRect(screen, &dst, sdl_color);
	return;

l:
	SDL_FillRect(screen, &dst1, sdl_color);
	SDL_FillRect(screen, &dst2, sdl_color);
}

/*
 * The timer callback executes on a separate thread.  To avoid multithreading
 * problems, push a "timer" event into the event queue and handle it in the
 * event loop.
 */
uint32_t timer_callback(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	//userevent.code = 0;
	//userevent.data1 = &my_function;
	//userevent.data2 = param;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return interval;
}

void draw_board(void)
{
	for (int y=0; y<16; y++) {
		for (int x=0; x<16; x++) {
			draw_tile(y, x, anim_phase);
		}
	}
	if (tank_alive) {
		draw_tank();
	}
	for (int i=0; i<num_lasers; i++) {
		draw_laser(&lasers[i]);
	}
}

void event_loop(void)
{
	SDL_Event e;

	for (;;) {
		SDL_WaitEvent(&e);
		switch (e.type) {
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
				int a;
			case SDLK_UP:
				a = tank_orient == NORTH ? MOVE_UP : TURN_UP;
				goto l;
			case SDLK_DOWN:
				a = tank_orient == SOUTH ? MOVE_DOWN : TURN_DOWN;
				goto l;
			case SDLK_LEFT:
				a = tank_orient == WEST ? MOVE_LEFT : TURN_LEFT;
				goto l;
			case SDLK_RIGHT:
				a = tank_orient == EAST ? MOVE_RIGHT : TURN_RIGHT;
				goto l;
			case SDLK_SPACE:
				a = FIRE;
l:
				try_set_tank_action(a);
				break;
			case SDLK_a:
				if (anim_on) {
					anim_on = false;
					anim_delay = 0;
					anim_phase = 0;
				} else {
					anim_on = true;
				}
				break;
			case SDLK_r:
				start_level();
				break;
			case SDLK_F9:
				if (editor_on) {
					close_editor();
				} else {
					open_editor();
				}
				draw_gui();
				render();
				break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button) {
				//int b;
			case SDL_BUTTON_LEFT:
				handle_button_down(&e.button);
#if 0
				b = get_button_at(e.button.x, e.button.y);
				switch (b) {
				case BUTTON_PREV_LEVEL:
					if (current_level > 0) {
						current_level--;
						start_level();
					}
					break;
				case BUTTON_NEXT_LEVEL:
					if (current_level < num_levels) {
						current_level++;
						start_level();
					}
					break;
				}
#endif
				break;
			case SDL_BUTTON_RIGHT:
				break;
			}
			break;
		case SDL_QUIT:
			return;
		case SDL_USEREVENT:
			if (active) {
				tick();
				if (anim_on) {
					anim_delay = (anim_delay-1)&3;
					if (anim_delay == 0) {
						anim_phase = (anim_phase+1)%3;
					}
				}
				draw_board();
				render();
			}
			break;
		}
	}
}

#define NUM_BUTTONS 9

void null_proc() {}

void hello()
{
	printf("hello\n");
}

void populate_gui(void)
{
	static SDL_Rect button_rect[NUM_BUTTONS] = {
		{ 5, 5, 70, 20 },
		{ 80, 5, 70, 20 },
		{ 5, 30, 145, 20 },
		{ 5, 55, 145, 20 },
		{ 5, 80, 70, 20 },
		{ 80, 80, 70, 20 },
		{ 5, 105, 145, 20 },
		{ 5, 130, 70, 20 },
		{ 80, 130, 70, 20 },
	};

	static void *button_cb[NUM_BUTTONS] = {
		null_proc,
		null_proc,
		null_proc,
		null_proc,
		null_proc,
		null_proc,
		null_proc,
		hello,
		hello,
	};

	static int palette_sprite_id[NUM_PALETTE_SPRITES] = {
		0,1,5,8,12,
		13,14,15,35,38,
		41,19,20,21,22,
		23,26,29,32,44,
		46,47,48,49,55,
		56,
	};
	static SDL_Surface *palette_sprites[NUM_PALETTE_SPRITES];

	struct button *b;

	// control panel absolute coordinates (bevel included): 560 250 715 405

	game_panel = malloc(sizeof *game_panel);
	panel_init(game_panel, 1, BEVEL_NONE);
	game_panel->w = 181;
	game_panel->h = 299;
	add_child(&root, WIDGET(game_panel), 550, 245);
	control_panel = malloc(sizeof *control_panel);
	panel_init(control_panel, NUM_BUTTONS, BEVEL_INNER);
	control_panel->w = 155;
	control_panel->h = 155;
	add_child(CONTAINER(game_panel), WIDGET(control_panel), 10, 5);
	for (int i=0; i<NUM_BUTTONS; i++) {
		b = malloc(sizeof *b);
		button_init(b, (button_down_handler) button_cb[i]);
		b->w = button_rect[i].w;
		b->h = button_rect[i].h;
		add_child(CONTAINER(control_panel), WIDGET(b), button_rect[i].x, button_rect[i].y);
	}

	for (int i=0; i<NUM_PALETTE_SPRITES; i++) {
		palette_sprites[i] = sprites[palette_sprite_id[i]];
	}

	editor_palette = malloc(sizeof *editor_palette);
	palette_init(editor_palette, 5, NUM_PALETTE_SPRITES, palette_sprites);
}

int main(int argc, char **argv)
{
	// same as data/bg.bmp
	int w = WINDOW_W;
	int h = WINDOW_H;
	SDL_Surface *bg;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n",
			SDL_GetError());
		return 1;
	}

	/*
	 * Load resources.
	 */
	bg = util_LoadBMP("data/bg.bmp");
	if (bg == NULL) {
		return 1;
	}
	if (load_level_set("data/LaserTank.lvl") < 0) {
		return 1;
	}
	if (load_graphic_set("data/default.ltg") < 0) {
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Laser Tank",
					      SDL_WINDOWPOS_UNDEFINED,
					      SDL_WINDOWPOS_UNDEFINED,
					      w, h, 0);
	if (window == NULL) {
		fprintf(stderr, "Unable to create main window: %s\n",
			SDL_GetError());
		return 1;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) {
		fprintf(stderr, "Unable to create renderer: %s\n",
			SDL_GetError());
		return 1;
	}

	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	screen = SDL_CreateRGBSurface(0, w, h, 32,
				      0x00FF0000,
				      0x0000FF00,
				      0x000000FF,
				      0xFF000000);
	if (screen == NULL) {
		return 1;
	}

	texture = SDL_CreateTexture(renderer,
				    SDL_PIXELFORMAT_ARGB8888,
				    SDL_TEXTUREACCESS_STREAMING,
				    w, h);
	if (texture == NULL) {
		return 1;
	}

	init_gui(1);
	populate_gui();

	SDL_AddTimer(50, timer_callback, NULL);

	// draw background
	SDL_BlitSurface(bg, NULL, screen, NULL);

	draw_gui();

	anim_on = true;
	current_level = 0;
	start_level();
	draw_board();

	render();

	event_loop();

	SDL_Quit();

	return 0;
}
