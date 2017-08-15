struct level {
	uint8_t board[16][16];
	char name[31];
	char hint[256];
	char author[31];
	uint8_t difficulty;
	uint8_t score;
};

extern int num_levels;
extern struct level *levels;

int load_level_set(const char *path);
