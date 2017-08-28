CFLAGS := -Wall -g -I/usr/local/include
CXXFLAGS := -std=c++11 $(CFLAGS)
TARGET := ltank
OBJECTS := game.o gui.o level.o ltg.o main.o util.o

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_ttf

game.o: game.c game.h
gui.o: gui.cpp gui.h
level.o: level.c level.h
ltg.o: ltg.c
main.o: main.cpp game.h gui.h level.h
util.o: util.c

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
