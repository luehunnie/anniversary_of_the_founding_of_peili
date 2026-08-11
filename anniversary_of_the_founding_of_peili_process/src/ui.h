#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* ── Button ─────────────────────────────────── */

typedef struct {
    SDL_Rect  rect;
    const char *text;
    SDL_Color normal_color;
    SDL_Color hover_color;
    SDL_Color text_color;
    SDL_Color border_color;
    int       has_border;
    int       hovered;
} Button;

void button_init(Button *b, int x, int y, int w, int h, const char *text);
void button_init_gold(Button *b, int x, int y, int w, int h, const char *text);
int  button_handle_event(Button *b, SDL_Event *e);
void button_render(Button *b, SDL_Renderer *r, TTF_Font *font);

/* ── TextInput ──────────────────────────────── */

#define TEXTINPUT_MAX 256

typedef struct {
    SDL_Rect rect;
    char     buf[TEXTINPUT_MAX + 1];
    int      len;
    int      active;
    int      cursor_visible;
    Uint32   cursor_timer;
    SDL_Color bg_color;
    SDL_Color border_color;
    SDL_Color active_color;
    SDL_Color text_color;
} TextInput;

void textinput_init(TextInput *t, int x, int y, int w, int h);
void textinput_handle_event(TextInput *t, SDL_Event *e);
void textinput_update(TextInput *t, Uint32 dt_ms);
void textinput_render(TextInput *t, SDL_Renderer *r, TTF_Font *font);

/* ── Hint Overlay ───────────────────────────── */

typedef struct {
    int       shown;
    char      message[256];
    Button    back_btn;
} HintOverlay;

void hint_overlay_init(HintOverlay *h);
void hint_overlay_show(HintOverlay *h, const char *msg);
void hint_overlay_hide(HintOverlay *h);
int  hint_overlay_handle_event(HintOverlay *h, SDL_Event *e);
void hint_overlay_render(HintOverlay *h, SDL_Renderer *r, TTF_Font *font);

/* ── Drawing helpers ────────────────────────── */

void draw_rounded_rect(SDL_Renderer *r, const SDL_Rect *rect, int radius, SDL_Color color);
void draw_rounded_rect_border(SDL_Renderer *r, const SDL_Rect *rect, int radius, SDL_Color color, int thickness);

#endif
