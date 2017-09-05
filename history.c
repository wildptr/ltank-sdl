#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"

#define HISTORY_SIZE 4096

static uint8_t *history;
static int hcap, hpos;

enum {
	MOVE_TANK_N,
	MOVE_TANK_E,
	MOVE_TANK_S,
	MOVE_TANK_W,
	MOVE_OBJ_N,
	MOVE_OBJ_E,
	MOVE_OBJ_S,
	MOVE_OBJ_W,
	BREAK_BRICK,
	BREAK_ANTI,
	SINK,
	DIE,
};

static uint8_t pack(int y, int x)
{
	return ((y&15)<<4)|(x&15);
}

static void unpack(uint8_t c, struct point *p)
{
	p->y = (c>>4)&15;
	p->x = c&15;
}

static void record(uint8_t c)
{
	if (hpos == hcap) {
		hcap *= 2;
		history = realloc(history, hcap);
	}
	history[hpos++] = c;
}

void record_move_tank(int y, int x, int dir)
{
	record(pack(y, x));
	record(MOVE_TANK_N + dir);
}

void record_move_object(int y, int x, int dir)
{
	record(pack(y, x));
	record(MOVE_OBJ_N + dir);
}

void record_break_brick(int y, int x)
{
	record(pack(y, x));
	record(BREAK_BRICK);
}

void record_break_anti(int y, int x)
{
	record(pack(y, x));
	record(BREAK_ANTI);
}

void record_sink(int y, int x, uint8_t obj)
{
	record(obj);
	record(pack(y, x));
	record(SINK);
}

void record_die(void)
{
	record(DIE);
}

extern int Dx[4], Dy[4];

void init_history(void)
{
	hcap = 0x1000;
	history = malloc(hcap);
	hpos = 0;
}

void clear_history(void)
{
	hpos = 0;
}

void undo(void)
{
	int p = hpos-1;
	if (p < 0) return;
	while (p >= 0 && history[p] > MOVE_TANK_W)
		p--;
	int q = hpos;
	while (q > p) {
		uint8_t code = history[--q];
		struct point pt;
		if (code < DIE)
			unpack(history[--q], &pt);
		int y = pt.y;
		int x = pt.x;
		int d;
		switch (code) {
		case MOVE_TANK_N:
		case MOVE_TANK_E:
		case MOVE_TANK_S:
		case MOVE_TANK_W:
			tank_y = y;
			tank_x = x;
			tank_orient = code - MOVE_TANK_N;
			// this is necessary to prevent problems with tank
			// movers
			tank_action = 0;
			break;
		case BREAK_BRICK:
			board[y][x].fg = F_BRICK;
			break;
		case BREAK_ANTI:
			board[y][x].fg -= 4;
			break;
		case SINK:
			board[y][x].fg = history[--q];
			board[y][x].bg = B_WATER;
			break;
		case MOVE_OBJ_N:
		case MOVE_OBJ_E:
		case MOVE_OBJ_S:
		case MOVE_OBJ_W:
			d = code - MOVE_OBJ_N;
			board[y][x].fg = board[y+Dy[d]][x+Dx[d]].fg;
			board[y+Dy[d]][x+Dx[d]].fg = 0;
			break;
		case DIE:
			tank_alive = true;
			break;
		}
	}
	hpos = q;
	num_lasers = 0;
}
