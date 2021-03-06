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
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

extern "C" {
#include "game.h"
#include "history.h"
#include "level.h"
}

#include "gui.h"

// pixel offset of game board from top-left corner of window
#define BOARD_X 17
#define BOARD_Y 17

#define WINDOW_W 734
#define WINDOW_H 547

#define NUM_PALETTE_SPRITES 26

#define MS_PER_TICK 50

extern "C" int load_graphic_set(const char *path);

SDL_Renderer *renderer;
SDL_Texture *sprites[60];
SDL_Texture *bg_tex;
TTF_Font *font;

int num_levels;
struct level *levels;
int current_level;

bool anim_on;
int anim_delay;
int anim_phase;

/*
 * GUI widgets
 */
ImageWidget *bg_image_widget;
Canvas *board_canvas;
Panel *game_panel;
Panel *control_panel;
Palette *editor_palette;
TextWidget *level_number_text, *level_name_text, *level_author_text;

bool editor_on;

bool editor_left_button_down;
bool editor_right_button_down;

SDL_TimerID game_timer;

void render(void)
{
    draw_gui();
    SDL_RenderPresent(renderer);
}

void open_editor(void)
{
    SDL_RemoveTimer(game_timer);
    game_panel->n_child_--;
    game_panel->add_child(editor_palette, 4, 14);
    editor_on = true;
}

Uint32 timer_callback(Uint32 interval, void *param);

void close_editor(void)
{
    game_panel->n_child_--;
    game_panel->add_child(control_panel, 10, 5);
    game_timer = SDL_AddTimer(MS_PER_TICK, timer_callback, NULL);
    editor_on = false;
}

void place_object(int obj, int y, int x)
{
    uint8_t fg = 0, bg = 0;
    switch (obj) {
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

void start_level(void)
{
    struct level *l = &levels[current_level];
    for (int y=0; y<16; y++) {
        for (int x=0; x<16; x++) {
            uint8_t b = l->board[x][y];
            place_object(b, y, x);
        }
    }
    tank_orient = NORTH;
    tank_action = 0;
    tank_alive = true;
    num_lasers = 0;
    active = true;
    clear_history();
    if (editor_on) {
        close_editor();
    }
    char tmp[12];
    sprintf(tmp, "%d", current_level+1);
    level_number_text->set_text(tmp);
    level_name_text->set_text(l->name);
    level_author_text->set_text(l->author);
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
    SDL_Texture *bg, *fg;

    SDL_Rect dst;
    dst.x = BOARD_X + x*32;
    dst.y = BOARD_Y + y*32;
    dst.w = 32;
    dst.h = 32;

    bg = sprites[get_sprite_id(t->bg, anim_phase)];
    SDL_RenderCopy(renderer, bg, NULL, &dst);
    if (t->fg) {
        fg = sprites[get_sprite_id(t->fg, anim_phase)];
        SDL_RenderCopy(renderer, fg, NULL, &dst);
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

    SDL_RenderCopy(renderer, sprites[sprite_id], NULL, &dst);
}

void draw_laser(const struct laser *l)
{
    int y = l->y;
    int x = l->x;

    int color = l->style >> 4;
    int shape = l->style & 15;

    uint8_t r = ((color>>0)&1)*255;
    uint8_t g = ((color>>1)&1)*255;
    uint8_t b = ((color>>2)&1)*255;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

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

    SDL_RenderFillRect(renderer, &dst);
    return;

l:
    SDL_RenderFillRect(renderer, &dst1);
    SDL_RenderFillRect(renderer, &dst2);
}

/*
 * The timer callback executes on a separate thread.  To avoid multithreading
 * problems, push a "timer" event into the event queue and handle it in the
 * event loop.
 */
Uint32 timer_callback(Uint32 interval, void *param)
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
    for (int i=0; i<num_visual_lasers; i++) {
        draw_laser(&visual_lasers[i]);
    }
    num_visual_lasers = 0;
}

void editor_place_object(int obj, int y, int x)
{
    //int old_tank_y = tank_y;
    //int old_tank_x = tank_x;
    place_object(obj, y, x);
    //draw_tile(y, x, 0);
    if (obj == 1 /* tank */) {
        tank_alive = true;
        //draw_tile(old_tank_y, old_tank_x, 0);
        //draw_tank();
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
                if (!editor_on) try_set_tank_action(a);
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
                if (!editor_on) start_level();
                break;
            case SDLK_u:
                if (!editor_on) undo();
                break;
            case SDLK_F9:
                if (editor_on) {
                    close_editor();
                } else {
                    open_editor();
                }
                render();
                break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            handle_button_down(&e.button);
            break;
        case SDL_MOUSEBUTTONUP:
            handle_button_up(&e.button);
            break;
        case SDL_MOUSEMOTION:
            handle_mouse_motion(&e.motion);
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
                render();
            }
            break;
        }
    }
}

#define NUM_BUTTONS 9

void null_proc(Widget *, int, int, int) {}

void cb_prev_level_button(Widget *, int, int, int)
{
    if (current_level > 0) {
        current_level--;
        start_level();
    }
}

void cb_next_level_button(Widget *, int, int, int)
{
    if (current_level < num_levels) {
        current_level++;
        start_level();
    }
}

void board_canvas_button_down(Widget *w, int button, int x, int y)
{
    switch (button) {
    case SDL_BUTTON_LEFT:
    case SDL_BUTTON_RIGHT:
        if (editor_on) {
            int cy = y >> 5;
            int cx = x >> 5;
            int obj;
            if (button == SDL_BUTTON_LEFT) {
                obj = editor_palette->selection1_;
                editor_left_button_down = true;
            } else {
                obj = editor_palette->selection2_;
                editor_right_button_down = true;
            }
            editor_place_object(obj, cy, cx);
            render();
            capture_mouse(w);
        } else {
        }
        break;
    }
}

void board_canvas_button_up(Widget *, int button, int x, int y)
{
    switch (button) {
    case SDL_BUTTON_LEFT:
        editor_left_button_down = false;
        break;
    case SDL_BUTTON_RIGHT:
        editor_right_button_down = false;
        break;
    }
    if (!editor_left_button_down && !editor_right_button_down)
        release_mouse();
}

void board_canvas_mouse_move(Widget *, int x, int y)
{
    if (editor_left_button_down || editor_right_button_down) {
        if (!((x|y)&-512)) {
            int cy = y >> 5;
            int cx = x >> 5;
            int obj;
            if (editor_left_button_down) {
                obj = editor_palette->selection1_;
            } else {
                obj = editor_palette->selection2_;
            }
            editor_place_object(obj, cy, cx);
            render();
        }
    }
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
        { 80, 105, 70, 20 },
        { 5, 130, 70, 20 },
        { 80, 130, 70, 20 },
    };

    static button_event_handler button_cb[NUM_BUTTONS] = {
        null_proc,
        null_proc,
        null_proc,
        null_proc,
        null_proc,
        null_proc,
        null_proc,
        cb_prev_level_button,
        cb_next_level_button,
    };

    static int palette_sprite_id[NUM_PALETTE_SPRITES] = {
        0,1,5,8,12,
        13,14,15,35,38,
        41,19,20,21,22,
        23,26,29,32,44,
        46,47,48,49,55,
        56,
    };
    static SDL_Texture *palette_sprites[NUM_PALETTE_SPRITES];

    static Button *button[NUM_BUTTONS];

    bg_image_widget = new ImageWidget(bg_tex);
    bg_image_widget->set_size(WINDOW_W, WINDOW_H);
    root->add_child(bg_image_widget, 0, 0);

    board_canvas = new Canvas();
    board_canvas->set_size(512, 512);
    board_canvas->button_down_ = board_canvas_button_down;
    board_canvas->button_up_ = board_canvas_button_up;
    board_canvas->mouse_move_ = board_canvas_mouse_move;
    board_canvas->paint_ = (paint_handler) draw_board;
    root->add_child(board_canvas, BOARD_X, BOARD_Y);

    level_number_text = new TextWidget();
    level_number_text->set_size(99, 21);
    level_number_text->set_text_color(255, 255, 255, 255);
    root->add_child(level_number_text, 592, 41);

    level_name_text = new TextWidget();
    level_name_text->set_size(163, 21);
    level_name_text->set_text_color(255, 255, 255, 255);
    root->add_child(level_name_text, 559, 99);

    level_author_text = new TextWidget();
    level_author_text->set_size(163, 21);
    level_author_text->set_text_color(255, 255, 255, 255);
    root->add_child(level_author_text, 559, 148);

    // control panel absolute coordinates (bevel included): 560 250 715 405

    game_panel = new Panel(1, Panel::BEVEL_NONE);
    game_panel->set_size(181, 299);
    root->add_child(game_panel, 550, 245);

    control_panel = new Panel(NUM_BUTTONS+1, Panel::BEVEL_INNER);
    control_panel->set_size(155, 155);
    game_panel->add_child(control_panel, 10, 5);

    for (int i=0; i<NUM_BUTTONS; i++) {
        Button *b = new Button(button_event_handler(button_cb[i]));
        button[i] = b;
        b->set_size(button_rect[i].w, button_rect[i].h);
        switch (i) {
        case 6:
            b->set_text("Go");
            break;
        case 7:
            b->set_text("<< Level");
            break;
        case 8:
            b->set_text("Level >>");
            break;
        }
        control_panel->add_child(b, button_rect[i].x, button_rect[i].y);
    }

    Edit *edit = new Edit();
    edit->set_size(70, 20);
    control_panel->add_child(edit, 5, 105);

    for (int i=0; i<NUM_PALETTE_SPRITES; i++) {
        palette_sprites[i] = sprites[palette_sprite_id[i]];
    }

    editor_palette = new Palette(5, NUM_PALETTE_SPRITES, palette_sprites);
}

SDL_Texture *load_texture(const char *path)
{
    SDL_Surface *s = IMG_Load(path);
    if (s == NULL) {
        fprintf(stderr, "Unable to load image %s: %s\n",
                path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, s);
    return tex;
}

int main(int argc, char **argv)
{
    // same as data/bg.bmp
    int w = WINDOW_W;
    int h = WINDOW_H;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n",
                SDL_GetError());
        return 1;
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "Unable to initialize SDL_ttf: %s\n",
                TTF_GetError());
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

    /*
     * Load resources (after creating renderer).
     */
    bg_tex = load_texture("data/bg.bmp");
    if (bg_tex == NULL) return 1;
    if (load_level_set("data/LaserTank.lvl") < 0) {
        return 1;
    }
    if (load_graphic_set("data/default.ltg") < 0) {
        return 1;
    }
    font = TTF_OpenFont("data/arial.ttf", 13);
    if (font == NULL) return 1;

    init_gui(6);
    populate_gui();

    anim_on = true;
    current_level = 0;
    init_history();
    start_level();

    game_timer = SDL_AddTimer(MS_PER_TICK, timer_callback, NULL);

    render();

    event_loop();

    TTF_Quit();
    SDL_Quit();

    return 0;
}
