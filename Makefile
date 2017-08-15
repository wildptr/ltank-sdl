CFLAGS := -Wall -g -I/usr/local/include
TARGET := ltank
OBJECTS := game.o gui.o level.o ltg.o main.o palette.o util.o

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ -L/usr/local/lib -lSDL2 -lSDL2_image

game.o: game.c game.h
gui.o: gui.c gui.h
level.o: level.c level.h
ltg.o: ltg.c
main.o: main.c game.h gui.h level.h
palette.o: palette.c gui.h palette.h
util.o: util.c

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
