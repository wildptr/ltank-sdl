#include <SDL2/SDL_image.h>
#include <stdint.h>

struct ltg_header {
	char name[40];
	char author[30];
	char info[245];
	char sig[5];
	int32_t mask_offset;
};

extern SDL_Surface *sprites[60];

int load_graphic_set(const char *path)
{
	static char has_mask[60] = {
		0,1,1,1,1,0,0,0,0,0,
		0,1,0,1,0,1,1,1,0,1,
		1,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,
		1,1,1,1,0,0,0,0,0,0,
		1,1,1,1,1,0,0,0,0,0,
	};

	FILE *fp;
	struct ltg_header header;
	long file_size;
	int buf_size;
	int color_bmp_size;
	int ret;
	SDL_RWops *color_rw, *mask_rw;
	SDL_Surface *color_surf, *mask_surf;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf_size = file_size - sizeof header;
	if (buf_size < 0) {
		ret = -1;
		goto cleanup1;
	}

	fread(&header, sizeof header, 1, fp);

	void *buf = malloc(file_size - sizeof header);
	if (buf == NULL) {
		ret = -1;
		goto cleanup1;
	}

	fread(buf, 1, file_size - sizeof header, fp);

	color_bmp_size = header.mask_offset - sizeof header;
	color_rw = SDL_RWFromMem(buf, color_bmp_size);
	mask_rw = SDL_RWFromMem(buf + color_bmp_size, file_size - header.mask_offset);

	color_surf = IMG_LoadBMP_RW(color_rw);
	if (color_surf == NULL) {
		ret = -1;
		goto cleanup2;
	}

	mask_surf = IMG_LoadBMP_RW(mask_rw);
	if (mask_surf == NULL) {
		ret = -1;
		goto cleanup3;
	}

	SDL_Rect src;
	src.w = 32;
	src.h = 32;

	SDL_Surface *tmp_surf = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);

	int i = 0;
	for (int y=0; y<6; y++) {
		for (int x=0; x<10; x++) {
			SDL_Surface *sprite = SDL_CreateRGBSurface
				(0, 32, 32, 32,
				 0x000000ff,
				 0x0000ff00,
				 0x00ff0000,
				 0xff000000);
			sprites[i] = sprite;
			SDL_SetSurfaceBlendMode(sprite, SDL_BLENDMODE_BLEND);
			src.x = x*32;
			src.y = y*32;
			SDL_BlitSurface(color_surf, &src, sprite, NULL);
			SDL_BlitSurface(mask_surf, &src, tmp_surf, NULL);
			if (has_mask[i]) {
				SDL_LockSurface(sprite);
				SDL_LockSurface(tmp_surf);
				uint8_t *c = sprite->pixels;
				uint8_t *m = tmp_surf->pixels;
				for (int yy=0; yy<32; yy++) {
					for (int xx=0; xx<32; xx++) {
						c[xx*4+3] = 255 - m[xx*4];
					}
					c += sprite->pitch;
					m += tmp_surf->pitch;
				}
				SDL_UnlockSurface(tmp_surf);
				SDL_UnlockSurface(sprite);
			}
			i++;
		}
	}

	SDL_FreeSurface(tmp_surf);

	ret = 0;

cleanup4:
	SDL_FreeRW(mask_rw);
cleanup3:
	SDL_FreeRW(color_rw);
cleanup2:
	free(buf);
cleanup1:
	fclose(fp);
	return ret;
}
