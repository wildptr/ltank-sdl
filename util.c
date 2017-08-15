#include <SDL2/SDL.h>

SDL_Surface *util_LoadBMP(const char *path)
{
	SDL_Surface *s = SDL_LoadBMP(path);
	if (s == NULL) {
		fprintf(stderr, "Unable to load BMP file %s: %s\n",
			path, SDL_GetError());
	}
	return s;
}
