#include <stdbool.h>
#include <stdint.h>

#include "game.h"

#define MAX_LASERS 256

struct tile board[16][16];
int tank_y, tank_x;
int tank_orient;
int tank_action;
bool tank_alive;
int num_lasers, num_visual_lasers;
struct laser lasers[MAX_LASERS], visual_lasers[256];
bool active;

static int Dx[4] = {0,1,0,-1};
static int Dy[4] = {-1,0,1,0};

struct laser *add_laser(int y, int x, int color, int dir)
{
	struct laser *l = &lasers[num_lasers++];

	l->y = y;
	l->x = x;
	l->dir = dir;

	l->style = color << 4 | (dir&1);

	return l;
}

bool check_antitank(int dir, struct point *p)
{
	int y = tank_y;
	int x = tank_x;

	switch (dir) {
	case NORTH:
		while (y-- > 0) {
			uint8_t obj = board[y][x].fg;
			if (obj) {
				if (obj == F_ANTI_S) {
					p->x = x;
					p->y = y;
					return true;
				}
				return false;
			}
		}
		return false;
	case EAST:
		while (x++ < 15) {
			uint8_t obj = board[y][x].fg;
			if (obj) {
				if (obj == F_ANTI_W) {
					p->x = x;
					p->y = y;
					return true;
				}
				return false;
			}
		}
		return false;
	case SOUTH:
		while (y++ < 15) {
			uint8_t obj = board[y][x].fg;
			if (obj) {
				if (obj == F_ANTI_N) {
					p->x = x;
					p->y = y;
					return true;
				}
				return false;
			}
		}
		return false;
	case WEST:
		while (x-- > 0) {
			uint8_t obj = board[y][x].fg;
			if (obj) {
				if (obj == F_ANTI_E) {
					p->x = x;
					p->y = y;
					return true;
				}
				return false;
			}
		}
		return false;
	}
	return false;
}

void fix_tile(struct tile *t)
{
	if (t->bg == B_WATER) {
		if (t->fg == F_CRATE) t->bg = B_BRIDGE;
		t->fg = 0;
	}
}

void try_nudge_object(int y, int x, int dir)
{
	int yy = y + Dy[dir];
	int xx = x + Dx[dir];

	if ((yy|xx)>>4 == 0 && board[yy][xx].fg == 0) {
		board[yy][xx].fg = board[y][x].fg;
		board[y][x].fg = 0;
		fix_tile(&board[yy][xx]);
	}
}

void fix_laser(struct laser *l)
{
	int y = l->y;
	int x = l->x;

	if (tank_alive) {
		if (y == tank_y && x == tank_x) {
			tank_alive = false;
			l->dir = -1;
			return;
		}
	}

	uint8_t obj = board[y][x].fg;
	switch (obj) {
	case F_WALL:
	case F_BLOWN_ANTI_N:
	case F_BLOWN_ANTI_E:
	case F_BLOWN_ANTI_S:
	case F_BLOWN_ANTI_W:
		l->dir = -1;
		break;
	case F_CRATE:
		try_nudge_object(y, x, l->dir);
		l->dir = -1;
		break;
	case F_BRICK:
		board[y][x].fg = 0;
		l->dir = -1;
		break;
	case F_ANTI_N:
	case F_ANTI_E:
	case F_ANTI_S:
	case F_ANTI_W:
		if (obj - F_ANTI_N == ((l->dir+2)&3)) {
			board[y][x].fg += 4;
		} else {
			try_nudge_object(y, x, l->dir);
		}
		l->dir = -1;
		break;
	case F_MIRROR_NE:
		switch (l->dir) {
		case SOUTH:
		case WEST:
			l->dir = (3-l->dir)&3;
			l->style = (l->style & 0xf0) | 2;
			break;
		default:
			try_nudge_object(y, x, l->dir);
			l->dir = -1;
		}
		break;
	case F_MIRROR_SE:
		switch (l->dir) {
		case WEST:
		case NORTH:
			l->dir = (1-l->dir)&3;
			l->style = (l->style & 0xf0) | 3;
			break;
		default:
			try_nudge_object(y, x, l->dir);
			l->dir = -1;
		}
		break;
	case F_MIRROR_SW:
		switch (l->dir) {
		case NORTH:
		case EAST:
			l->dir = (3-l->dir)&3;
			l->style = (l->style & 0xf0) | 4;
			break;
		default:
			try_nudge_object(y, x, l->dir);
			l->dir = -1;
		}
		break;
	case F_MIRROR_NW:
		switch (l->dir) {
		case EAST:
		case SOUTH:
			l->dir = (1-l->dir)&3;
			l->style = (l->style & 0xf0) | 5;
			break;
		default:
			try_nudge_object(y, x, l->dir);
			l->dir = -1;
		}
		break;
	}
}

bool is_clear(int y, int x, int dir)
{
	y += Dy[dir];
	x += Dx[dir];
	return (y|x)>>4 == 0 && board[y][x].fg == 0;
}

int tank_mover_direction(void)
{
	uint8_t bg = board[tank_y][tank_x].bg;

	switch (bg) {
		int dir;
	case B_BELT_N:
	case B_BELT_E:
	case B_BELT_S:
	case B_BELT_W:
		dir = bg - B_BELT_N;
		return is_clear(tank_y, tank_x, dir) ? dir : -1;
	}

	return -1;
}

void move_tank(int dir)
{
	tank_x += Dx[dir];
	tank_y += Dy[dir];
}

void update_laser(struct laser *l)
{
	switch (l->dir) {
	case NORTH:
		if (l->y > 0) l->y--;
		else l->dir = -1;
		break;
	case EAST:
		if (l->x < 15) l->x++;
		else l->dir = -1;
		break;
	case SOUTH:
		if (l->y < 15) l->y++;
		else l->dir = -1;
		break;
	case WEST:
		if (l->x > 0) l->x--;
		else l->dir = -1;
		break;
	}
	if (l->dir >= 0) {
		// straighten lasers bent by mirrors
		l->style = (l->style & 0xf0) | (l->dir&1);
		fix_laser(l);
	}
}

void tick(void)
{
	if (tank_action == 0 && num_lasers == 0) {
		int d = tank_mover_direction();
		if (d < 0) return;
	}

	/*
	 * Let the tank fire laser.
	 */
	if (tank_action == FIRE) {
		if (num_lasers < MAX_LASERS) {
			int y = tank_y;
			int x = tank_x;
			struct laser *l = add_laser(y, x, 2, tank_orient);
			// if instantaneous firing is not desired, comment out
			// this loop
			while (l->dir >= 0) {
				update_laser(l);
				if (l->dir >= 0 || !board[l->y][l->x].fg) {
					struct laser *vl =
						&visual_lasers[num_visual_lasers++];
					vl->y = l->y;
					vl->x = l->x;
					vl->style = l->style;
				}
			}
		}
		tank_action = 0;
	}

	/*
	 * Update lasers.
	 */
	for (int i=0; i<num_lasers; i++) {
		update_laser(&lasers[i]);
	}

	/*
	 * Update tank position.
	 */
	int d = tank_mover_direction();
	if (d >= 0) {
		move_tank(d);
		tank_action = 0;
	}

	switch (tank_action) {
		int dir;
	case TURN_UP:
	case TURN_RIGHT:
	case TURN_DOWN:
	case TURN_LEFT:
		tank_orient = tank_action - TURN_UP;
		break;
	case MOVE_UP:
	case MOVE_RIGHT:
	case MOVE_DOWN:
	case MOVE_LEFT:
		dir = tank_action - MOVE_UP;
		if (is_clear(tank_y, tank_x, dir)) {
			move_tank(dir);
		}
		break;
	}
	tank_action = 0;

	/*
	 * Update flags according to the object the tank is sitting on.
	 */
	uint8_t bg = board[tank_y][tank_x].bg;
	switch (bg) {
		int dir;
	case B_FLAG:
		active = false;
		break;
	case B_WATER:
		tank_alive = false;
		break;
	case B_BELT_N:
	case B_BELT_E:
	case B_BELT_S:
	case B_BELT_W:
		dir = bg - B_BELT_N;
		if (is_clear(tank_y, tank_x, dir)) {
			tank_action = MOVE_UP + dir;
		}
		break;
	}

	/*
	 * Check if tank is in sight of anti-tanks.
	 */
	if (tank_alive) {
		struct point anti;
		for (int d=0; d<4; d++) {
			if (check_antitank(d, &anti)) {
				if (num_lasers < MAX_LASERS) {
					struct laser *l = add_laser(anti.y, anti.x, 1, (d+2)&3);
					// in case laser hits player immediately
					update_laser(l);
				}
			}
		}
	}

	/*
	 * Remove lasers marked for removal.
	 */
	for (int i=0; i<num_lasers; i++) {
		if (lasers[i].dir < 0) {
			int r = i+1;
			int w = i;
			while (r < num_lasers) {
				lasers[w++] = lasers[r++];
				while (lasers[r].dir < 0) r++;
			}
			num_lasers = w;
		}
	}
}

void try_set_tank_action(int a)
{
	if (tank_alive && num_lasers == 0 && tank_action == 0 &&
	    tank_mover_direction() < 0)
	{
		tank_action = a;
	}
}
