void record_move_tank(int y, int x, int dir);
void record_move_object(int y, int x, int dir);
void record_break_brick(int y, int x);
void record_break_anti(int y, int x);
void record_sink(int y, int x, uint8_t obj);
void record_die(void);
void init_history(void);
void clear_history(void);
void undo(void);
