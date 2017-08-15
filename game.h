enum game_object {
	B_NOTHING,
	B_FLAG,
	B_WATER,
	B_BRIDGE,
	B_BELT_N,
	B_BELT_E,
	B_BELT_S,
	B_BELT_W,
	B_ICE,
	B_THIN_ICE,
	B_0A,
	B_0B,
	F_WALL,
	F_CRATE,
	F_BRICK,
	F_CRYSTAL,
	//F_TANK_N,
	//F_TANK_E,
	//F_TANK_S,
	//F_TANK_W,
	F_ANTI_N,
	F_ANTI_E,
	F_ANTI_S,
	F_ANTI_W,
	F_BLOWN_ANTI_N,
	F_BLOWN_ANTI_E,
	F_BLOWN_ANTI_S,
	F_BLOWN_ANTI_W,
	F_MIRROR_NE,
	F_MIRROR_SE,
	F_MIRROR_SW,
	F_MIRROR_NW,
	F_ROT_MIRROR_NE,
	F_ROT_MIRROR_SE,
	F_ROT_MIRROR_SW,
	F_ROT_MIRROR_NW,
};

// for tank_orient
enum { NORTH, EAST, SOUTH, WEST };

// for tank_action
enum {
	NONE,
	FIRE,
	TURN_UP,
	TURN_RIGHT,
	TURN_DOWN,
	TURN_LEFT,
	MOVE_UP,
	MOVE_RIGHT,
	MOVE_DOWN,
	MOVE_LEFT,
};

struct tile {
	uint8_t fg, bg;
};

struct point {
	int x;
	int y;
};

struct laser {
	uint8_t y, x;
	int8_t dir; // dir<0: marked for removal
	uint8_t style;
};

extern struct tile board[16][16];
extern int tank_y, tank_x;
extern int tank_orient;
extern int tank_action;
extern bool tank_alive;
extern int num_lasers;
extern struct laser lasers[];
extern bool active;

void tick(void);
void try_set_tank_action(int a);
