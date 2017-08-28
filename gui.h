class Widget;

typedef void (*button_down_handler)(Widget *w, int button, int x, int y);
typedef void (*paint_handler)(Widget *w, int x, int y);

class Widget {
public:
    int w_, h_;
    virtual void button_down(int button, int x, int y);
    virtual void paint(int x, int y);
    void set_size(int w, int h);
};

struct child {
    Widget *w;
    int x;
    int y;
};

class Container: public Widget {
public:
    int n_child_;
    child *children_;
    void add_child(Widget *child, int x, int y);
    Container(int cap);
    void paint(int x, int y) override;
    void button_down(int button, int x, int y) override;
};

class Panel: public Container {
public:
    enum Bevel {
        BEVEL_NONE,
        BEVEL_INNER,
        BEVEL_OUTER,
    } bevel_;
    Panel(int cap, Bevel bevel);
    void paint(int x, int y) override;
};

class ImageWidget: public Widget {
public:
    SDL_Texture *tex_;
    ImageWidget(SDL_Texture *tex);
    void paint(int x, int y) override;
};

class Canvas: public Widget {
public:
    button_down_handler button_down_;
    paint_handler paint_;
    void paint(int x, int y) override;
    void button_down(int button, int x, int y) override;
};

class TextArea {
    const char *text_;
    SDL_Texture *tex_;
    int tex_w_, tex_h_;
public:
    SDL_Color color_;
    int style_;
    void set_text(const char *);
    void paint(int x, int y, int w, int h);
    TextArea();
    void set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
};

class TextWidget: public Widget {
public:
    TextArea text_area_;
    void paint(int x, int y) override;
    void set_text(const char *);
    void set_text_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
};

class Button: public Widget {
public:
    button_down_handler button_down_;
    TextArea caption_;
    Button(button_down_handler cb);
    void paint(int x, int y) override;
    void button_down(int button, int x, int y) override;
    void set_text(const char *);
};

class Palette: public Widget {
public:
    int n_col_;
    int n_row_;
    int n_sprite_;
    SDL_Texture **sprites_;
    int selection1_, selection2_;
    void paint(int x, int y) override;
    void button_down(int button, int x, int y) override;
    Palette(int n_col, int n_sprite, SDL_Texture **sprites);
};

extern Container *root;
extern SDL_Renderer *renderer;

void init_gui(int root_cap);
void handle_button_down(SDL_MouseButtonEvent *e);
void draw_gui(void);
