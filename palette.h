struct palette {
	WIDGET_FIELDS;
	int n_col;
	int n_row;
	int n_sprite;
	SDL_Texture **sprites;
	int selection1, selection2;
};

void palette_init(struct palette *w, int n_col, int n_sprite, SDL_Texture **sprites);
