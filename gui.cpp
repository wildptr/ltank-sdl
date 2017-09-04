#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "gui.h"

#define BLACK		  0,  0,  0,255
#define DARK_GRAY	128,128,128,255
#define LIGHT_GRAY	192,192,192,255
#define GRAY_224  	224,224,224,255
#define WHITE		255,255,255,255

extern TTF_Font *font;

// After init_gui(), this is always non-null.
//static Widget *widget_under_cursor;
static Widget *widget_capturing_mouse;

static void draw_inner_bevel(SDL_Rect *r);
static void draw_outer_bevel(SDL_Rect *r);
static void draw_button_border(bool depressed, SDL_Rect *r);
static Widget *widget_at(int x, int y);
static void capture_mouse(Widget *w);
static void release_mouse();

Container *root;

/*
 * Widget class
 */

void Widget::set_size(int w, int h)
{
    w_ = w;
    h_ = h;
}

/*
 * Text area class
 */

void TextArea::paint(int x, int y, int w, int h)
{
    if (tex_ == NULL) return;
    int font_height = TTF_FontHeight(font);
    SDL_Rect dst;
    dst.x = x + (w-tex_w_)/2;
    dst.y = y + (h-font_height)/2;
    dst.w = tex_w_;
    dst.h = tex_h_;
    SDL_RenderCopy(renderer, tex_, NULL, &dst);
}

TextArea::TextArea()
{
    color_.r = 0;
    color_.g = 0;
    color_.b = 0;
    color_.a = 255;
    style_ = 0;
    tex_ = nullptr;
}

void TextArea::set_text(const char *text)
{
    if (tex_)
        SDL_DestroyTexture(tex_);
    if (text) {
        SDL_Surface *s = TTF_RenderText_Blended(font, text, color_);
        tex_ = SDL_CreateTextureFromSurface(renderer, s);
        tex_w_ = s->w;
        tex_h_ = s->h;
        SDL_FreeSurface(s);
    } else {
        tex_ = nullptr;
        tex_w_ = 0;
        tex_h_ = 0;
    }
}

void TextArea::set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    color_.r = r;
    color_.g = g;
    color_.b = b;
    color_.a = a;
}

/*
 * Container class
 */

void Container::button_down(int button, int x, int y)
{
    /*
     * Iterate over children in reverse order since last child is on top.
     */
    for (int i=n_child_-1; i>=0; i--) {
        Widget *cw = children_[i];
        int dx = x - (cw->x_ - x_);
        if (dx < 0 || dx >= cw->w_) continue;
        int dy = y - (cw->y_ - y_);
        if (dy < 0 || dy >= cw->h_) continue;
        cw->button_down(button, dx, dy);
        break;
    }
}

void Container::paint()
{
    for (int i=0; i<n_child_; i++)
        children_[i]->paint();
}

Container::Container(int cap)
{
    children_ = new Widget*[cap];
    n_child_ = 0;
}

void Container::add_child(Widget *cw, int x, int y)
{
    cw->x_ = x_ + x;
    cw->y_ = y_ + y;
    children_[n_child_++] = cw;
}

/*
 * Panel class
 */

void Panel::paint()
{
    SDL_Rect dst = { x_, y_, w_, h_ };
    SDL_SetRenderDrawColor(renderer, LIGHT_GRAY);
    SDL_RenderFillRect(renderer, &dst);

    switch (bevel_) {
    case BEVEL_NONE:
        break;
    case BEVEL_INNER:
        draw_inner_bevel(&dst);
        break;
    case BEVEL_OUTER:
        draw_outer_bevel(&dst);
        break;
    }

    Container::paint();
}

Panel::Panel(int cap, Bevel bevel): Container(cap), bevel_(bevel) {}

/*
 * Button class
 */

void Button::button_down(int button, int x, int y)
{
    if (button == SDL_BUTTON_LEFT) {
        depressed_ = true;
        left_button_down_ = true;
        //click_(this, button, x, y);
        paint();
        capture_mouse(this);
    }
}

void Button::button_up(int button, int x, int y)
{
    if (left_button_down_ && button == SDL_BUTTON_LEFT) {
        left_button_down_ = false;
        if (depressed_) {
            depressed_ = false;
            click_(this, button, x, y);
        }
        paint();
        release_mouse();
    }
}

void Button::mouse_move(int x, int y)
{
    if (left_button_down_) {
        bool depressed_now = x >= 0 && x < w_ && y >= 0 && y < h_;
        if (depressed_ != depressed_now) {
            depressed_ = depressed_now;
            paint();
        }
    }
}

#if 0
void Button::mouse_leave()
{
    if (left_button_down_) {
        depressed_ = false;
        paint();
    }
}

void Button::mouse_enter()
{
    if (left_button_down_) {
        depressed_ = true;
        paint();
    }
}
#endif

void Button::paint()
{
    SDL_Rect r = { x_, y_, w_, h_ };
    draw_button_border(depressed_, &r);
    int d = depressed_ ? 1 : 0;
    caption_.paint(x_+d, y_+d, w_, h_);
}

Button::Button(button_down_handler cb):
    depressed_(false),
    left_button_down_(false),
    click_(cb) {}

void Button::set_text(const char *s)
{
    caption_.set_text(s);
}

/*
 * Image widget class
 */

void ImageWidget::paint()
{
    SDL_Rect dst = { x_, y_, w_, h_ };
    SDL_RenderCopy(renderer, tex_, NULL, &dst);
}

ImageWidget::ImageWidget(SDL_Texture *tex): tex_(tex) {}

/*
 * Canvas class
 */

void Canvas::button_down(int button, int x, int y)
{
    button_down_(this, button, x, y);
}

void Canvas::paint()
{
    paint_(this);
}

/*
 * Text widget class
 */

void TextWidget::paint()
{
    text_area_.paint(x_, y_, w_, h_);
}

void TextWidget::set_text(const char *s)
{
    text_area_.set_text(s);
}

void TextWidget::set_text_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    text_area_.set_color(r, g, b, a);
}

/*
 * Interface functions
 */

void init_gui(int root_cap)
{
    root = new Container(root_cap);
#if 0
    widget_under_cursor = root;
#endif
}

void draw_gui()
{
    root->paint();
}

void handle_button_down(SDL_MouseButtonEvent *e)
{
    Widget *w = widget_at(e->x, e->y);
    w->button_down(e->button, e->x - w->x_, e->y - w->y_);
}

void handle_button_up(SDL_MouseButtonEvent *e)
{
    Widget *w;
    if (widget_capturing_mouse) {
        w = widget_capturing_mouse;
    } else {
        w = widget_at(e->x, e->y);
    }
    w->button_up(e->button, e->x - w->x_, e->y - w->y_);
}

void handle_mouse_motion(SDL_MouseMotionEvent *e)
{
    Widget *w;
    if (widget_capturing_mouse) {
        w = widget_capturing_mouse;
    } else {
        w = widget_at(e->x, e->y);
    }
    w->mouse_move(e->x - w->x_, e->y - w->y_);
#if 0
    if (widget_under_cursor != w) {
        widget_under_cursor->mouse_leave();
        w->mouse_enter();
    }
    widget_under_cursor = w;
#endif
}

/*
 * Internal functions
 */

static void draw_inner_bevel(SDL_Rect *r)
{
    SDL_Point pt[3];

    int x0 = r->x;
    int y0 = r->y;
    int x1 = x0 + r->w;
    int y1 = y0 + r->h;

    pt[0].x = x0;
    pt[0].y = y1-2;
    pt[1].x = x0;
    pt[1].y = y0;
    pt[2].x = x1-2;
    pt[2].y = y0;

    SDL_SetRenderDrawColor(renderer, DARK_GRAY);
    SDL_RenderDrawLines(renderer, pt, 3);

    pt[0].x = x1-1;
    pt[0].y = y0;
    pt[1].x = x1-1;
    pt[1].y = y1-1;
    pt[2].x = x0;
    pt[2].y = y1-1;

    SDL_SetRenderDrawColor(renderer, WHITE);
    SDL_RenderDrawLines(renderer, pt, 3);
}

static void draw_outer_bevel(SDL_Rect *r)
{
    SDL_Point pt[3];

    int x0 = r->x;
    int y0 = r->y;
    int x1 = x0 + r->w;
    int y1 = y0 + r->h;

    pt[0].x = x0;
    pt[0].y = y1-2;
    pt[1].x = x0;
    pt[1].y = y0;
    pt[2].x = x1-2;
    pt[2].y = y0;

    SDL_SetRenderDrawColor(renderer, WHITE);
    SDL_RenderDrawLines(renderer, pt, 3);

    pt[0].x = x1-1;
    pt[0].y = y0;
    pt[1].x = x1-1;
    pt[1].y = y1-1;
    pt[2].x = x0;
    pt[2].y = y1-1;

    SDL_SetRenderDrawColor(renderer, DARK_GRAY);
    SDL_RenderDrawLines(renderer, pt, 3);
}

/*
 * Palette class
 */

void Palette::button_down(int button, int x, int y)
{
    int cell_x = x/36;
    int cell_y = y/36;

    int sel = cell_y * n_col_ + cell_x;

    switch (button) {
    case SDL_BUTTON_LEFT:
        selection1_ = sel;
        break;
    case SDL_BUTTON_RIGHT:
        selection2_ = sel;
        break;
    }
}

void Palette::paint()
{
    SDL_Rect dst, bevel;
    dst.w = 32;
    dst.h = 32;
    bevel.w = 34;
    bevel.h = 34;
    int i = 0;

    for (int cell_y = 0; cell_y < n_row_; cell_y++) {
        for (int cell_x = 0; cell_x < n_col_; cell_x++) {
            if (i >= n_sprite_) break;
            bevel.x = x_ + 36*cell_x;
            bevel.y = y_ + 36*cell_y;
            dst.x = bevel.x + 1;
            dst.y = bevel.y + 1;
            SDL_RenderCopy(renderer, sprites_[i], NULL, &dst);
            draw_outer_bevel(&bevel);
            i++;
        }
    }
}

Palette::Palette(int n_col, int n_sprite, SDL_Texture **sprites)
{
    int n_row = (n_sprite+n_col-1)/n_col;
    w_ = 36*n_col-4;
    h_ = 36*n_row-4;
    n_col_ = n_col;
    n_row_ = n_row;
    n_sprite_ = n_sprite;
    sprites_ = sprites;
    selection1_ = 0;
    selection2_ = 0;
}

static void draw_button_border(bool depressed, SDL_Rect *rect)
{
    int x0 = rect->x;
    int y0 = rect->y;
    int x1 = rect->x + rect->w;
    int y1 = rect->y + rect->h;

    SDL_Point pt[3];
    SDL_Rect r;

    pt[0].x = x0;
    pt[0].y = y1-2;
    pt[1].x = x0;
    pt[1].y = y0;
    pt[2].x = x1-2;
    pt[2].y = y0;

    if (depressed) {
        SDL_SetRenderDrawColor(renderer, BLACK);
    } else {
        SDL_SetRenderDrawColor(renderer, WHITE);
    }
    SDL_RenderDrawLines(renderer, pt, 3);

    pt[0].x = x0+1;
    pt[0].y = y1-3;
    pt[1].x = x0+1;
    pt[1].y = y0+1;
    pt[2].x = x1-3;
    pt[2].y = y0+1;

    if (depressed) {
        SDL_SetRenderDrawColor(renderer, DARK_GRAY);
    } else {
        SDL_SetRenderDrawColor(renderer, GRAY_224);
    }
    SDL_RenderDrawLines(renderer, pt, 3);

    r.x = x0+2;
    r.y = y0+2;
    r.w = rect->w-4;
    r.h = rect->h-4;

    SDL_SetRenderDrawColor(renderer, LIGHT_GRAY);
    SDL_RenderFillRect(renderer, &r);

    pt[0].x = x0+1;
    pt[0].y = y1-2;
    pt[1].x = x1-2;
    pt[1].y = y1-2;
    pt[2].x = x1-2;
    pt[2].y = y0+1;

    if (depressed) {
        SDL_SetRenderDrawColor(renderer, GRAY_224);
    } else {
        SDL_SetRenderDrawColor(renderer, DARK_GRAY);
    }
    SDL_RenderDrawLines(renderer, pt, 3);

    pt[0].x = x1-1;
    pt[0].y = y0;
    pt[1].x = x1-1;
    pt[1].y = y1-1;
    pt[2].x = x0;
    pt[2].y = y1-1;

    if (depressed) {
        SDL_SetRenderDrawColor(renderer, WHITE);
    } else {
        SDL_SetRenderDrawColor(renderer, BLACK);
    }
    SDL_RenderDrawLines(renderer, pt, 3);
}

static Widget *widget_at(int x, int y)
{
    Container *c = root;
loop:
    for (int i=c->n_child_-1; i>=0; i--) {
        Widget *cw = c->children_[i];
        int dx = x - cw->x_;
        if (dx < 0 || dx >= cw->w_) continue;
        int dy = y - cw->y_;
        if (dy < 0 || dy >= cw->h_) continue;
        Container *cc = dynamic_cast<Container*>(cw);
        if (cc) {
            c = cc;
            goto loop;
        } else {
            return cw;
        }
    }
    return c;
}

static void capture_mouse(Widget *w)
{
    //printf("capture mouse: %p\n", w);
    widget_capturing_mouse = w;
    SDL_CaptureMouse(SDL_TRUE);
}

static void release_mouse()
{
    //puts("release mouse");
    widget_capturing_mouse = nullptr;
    SDL_CaptureMouse(SDL_FALSE);
}
