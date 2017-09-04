class Widget;

typedef void (*button_down_handler)(Widget *w, int button, int x, int y);
typedef void (*paint_handler)(Widget *w);

class Widget {
public:
    int x_, y_, w_, h_;
    virtual void button_down(int button, int x, int y) {}
    virtual void button_up(int button, int x, int y) {}
    //virtual void mouse_leave() {}
    //virtual void mouse_enter() {}
    virtual void mouse_move(int x, int y) {}
    virtual void paint() {}
    void set_size(int w, int h);
    Widget(): x_(0), y_(0), w_(0), h_(0) {}
    virtual ~Widget() {}
};

class Container: public Widget {
public:
    int n_child_;
    Widget **children_;
    int cap_;
    void add_child(Widget *child, int x, int y);
    Container(int cap);
    void paint() override;
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
    void paint() override;
};

class ImageWidget: public Widget {
public:
    SDL_Texture *tex_;
    ImageWidget(SDL_Texture *tex);
    void paint() override;
};

class Canvas: public Widget {
public:
    button_down_handler button_down_;
    paint_handler paint_;
    void paint() override;
    void button_down(int button, int x, int y) override;
};

class TextArea {
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
    void paint() override;
    void set_text(const char *);
    void set_text_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
};

class Button: public Widget {
    bool depressed_;
    bool left_button_down_;
public:
    button_down_handler click_;
    TextArea caption_;
    Button(button_down_handler cb);
    void paint() override;
    void button_down(int button, int x, int y) override;
    void button_up(int button, int x, int y) override;
    //void mouse_leave() override;
    //void mouse_enter() override;
    void mouse_move(int x, int y) override;
    void set_text(const char *);
};

class Palette: public Widget {
public:
    int n_col_;
    int n_row_;
    int n_sprite_;
    SDL_Texture **sprites_;
    int selection1_, selection2_;
    void paint() override;
    void button_down(int button, int x, int y) override;
    Palette(int n_col, int n_sprite, SDL_Texture **sprites);
};

extern Container *root;
extern SDL_Renderer *renderer;

void init_gui(int root_cap);
void handle_button_down(SDL_MouseButtonEvent *e);
void handle_button_up(SDL_MouseButtonEvent *e);
void handle_mouse_motion(SDL_MouseMotionEvent *e);
void draw_gui(void);
