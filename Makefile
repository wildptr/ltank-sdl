CFLAGS := -Wall -g -I/usr/local/include
TARGET := ltank
OBJECTS := game.o gui.o level.o main.o ltg.o util.o

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ -L/usr/local/lib -lSDL2 -lSDL2_image

game.o: game.c game.h
gui.o: gui.c gui.h
level.o: level.c level.h
main.o: main.c game.h gui.h level.h
ltg.o: ltg.c
util.o: util.c

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
