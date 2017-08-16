#define WIDGET(x) ((struct widget *)(x))
#define CONTAINER(x) ((struct container *)(x))

struct widget;

typedef void (*button_down_handler)(struct widget *w, int button, int x, int y);
typedef void (*paint_handler)(struct widget *w, int x, int y);

struct widget_class {
	button_down_handler button_down;
	paint_handler paint;
};

#define WIDGET_FIELDS \
struct { \
	int w, h; \
	struct widget_class *class; \
}

#define CONTAINER_FIELDS \
struct { \
	WIDGET_FIELDS; \
	int n_child; \
	struct child *children; \
}

struct widget {
	WIDGET_FIELDS;
};

struct child {
	struct widget *w;
	int x;
	int y;
};

struct container {
	CONTAINER_FIELDS;
};

struct panel {
	CONTAINER_FIELDS;
	enum {
		BEVEL_NONE,
		BEVEL_INNER,
		BEVEL_OUTER,
	} bevel;
};

struct button {
	WIDGET_FIELDS;
	button_down_handler button_down;
};

struct image_widget {
	WIDGET_FIELDS;
	SDL_Texture *tex;
};

struct canvas {
	WIDGET_FIELDS;
	button_down_handler button_down;
	paint_handler paint;
};

extern struct container root;
extern struct SDL_Renderer *renderer;

void init_gui(int root_cap);
void handle_button_down(SDL_MouseButtonEvent *e);
void draw_gui(void);
void add_child(struct container *parent, struct widget *child, int x, int y);
void widget_init(struct widget *w, struct widget_class *class);
void container_init(struct container *w, int cap);
void panel_init(struct panel *w, int cap, int bevel);
void button_init(struct button *w, button_down_handler cb);
void image_widget_init(struct image_widget *w, SDL_Texture *tex);
void canvas_init(struct canvas *s);
void draw_inner_bevel(SDL_Rect *r);
void draw_outer_bevel(SDL_Rect *r);
