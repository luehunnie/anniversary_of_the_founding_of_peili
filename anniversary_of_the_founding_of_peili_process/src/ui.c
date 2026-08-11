#include "ui.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ── Drawing helpers ────────────────────────── */

static void fill_circle(SDL_Renderer *r, int cx, int cy, int radius)
{
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)sqrtf((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void draw_rounded_rect(SDL_Renderer *r, const SDL_Rect *rect, int radius, SDL_Color c)
{
    int x = rect->x, y = rect->y, w = rect->w, h = rect->h;
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;

    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &(SDL_Rect){x + radius, y, w - 2 * radius, h});
    SDL_RenderFillRect(r, &(SDL_Rect){x, y + radius, radius, h - 2 * radius});
    SDL_RenderFillRect(r, &(SDL_Rect){x + w - radius, y + radius, radius, h - 2 * radius});
    fill_circle(r, x + radius,     y + radius,     radius);
    fill_circle(r, x + w - radius, y + radius,     radius);
    fill_circle(r, x + radius,     y + h - radius, radius);
    fill_circle(r, x + w - radius, y + h - radius, radius);
}

void draw_rounded_rect_border(SDL_Renderer *r, const SDL_Rect *rect, int radius, SDL_Color c, int thickness)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    int x = rect->x, y = rect->y, w = rect->w, h = rect->h;

    for (int t = 0; t < thickness; t++) {
        int tx = x + t, ty = y + t, tw = w - 2 * t, th = h - 2 * t;
        if (tw <= 0 || th <= 0) break;
        int tr = radius - t;
        if (tr < 1) tr = 1;

        /* Horizontal lines */
        SDL_RenderDrawLine(r, tx + tr, ty, tx + tw - tr, ty);
        SDL_RenderDrawLine(r, tx + tr, ty + th - 1, tx + tw - tr, ty + th - 1);
        /* Vertical lines */
        SDL_RenderDrawLine(r, tx, ty + tr, tx, ty + th - tr);
        SDL_RenderDrawLine(r, tx + tw - 1, ty + tr, tx + tw - 1, ty + th - tr);

        /* Corner arcs (approximate) */
        for (int a = 0; a < 90; a++) {
            float rad = a * 3.14159265f / 180.0f;
            int dx = (int)(tr * cosf(rad));
            int dy = (int)(tr * sinf(rad));
            SDL_RenderDrawPoint(r, tx + tr - dx,        ty + tr - dy);
            SDL_RenderDrawPoint(r, tx + tw - tr - 1 + dx, ty + tr - dy);
            SDL_RenderDrawPoint(r, tx + tr - dx,        ty + th - tr - 1 + dy);
            SDL_RenderDrawPoint(r, tx + tw - tr - 1 + dx, ty + th - tr - 1 + dy);
        }
    }
}

/* ── Button ─────────────────────────────────── */

static const SDL_Color BTN_NORMAL  = {70, 130, 180, 255};
static const SDL_Color BTN_HOVER   = {100, 160, 210, 255};
static const SDL_Color BTN_TEXT    = {255, 255, 255, 255};
static const SDL_Color GOLD        = {255, 215, 0, 255};

static const SDL_Color GOLD_BTN_NORMAL  = {40, 20, 10, 220};
static const SDL_Color GOLD_BTN_HOVER   = {80, 50, 20, 240};
static const SDL_Color GOLD_BTN_TEXT    = {255, 215, 0, 255};

void button_init(Button *b, int x, int y, int w, int h, const char *text)
{
    b->rect         = (SDL_Rect){x, y, w, h};
    b->text         = text;
    b->normal_color = BTN_NORMAL;
    b->hover_color  = BTN_HOVER;
    b->text_color   = BTN_TEXT;
    b->border_color = GOLD;
    b->has_border   = 0;
    b->hovered      = 0;
}

void button_init_gold(Button *b, int x, int y, int w, int h, const char *text)
{
    b->rect         = (SDL_Rect){x, y, w, h};
    b->text         = text;
    b->normal_color = GOLD_BTN_NORMAL;
    b->hover_color  = GOLD_BTN_HOVER;
    b->text_color   = GOLD_BTN_TEXT;
    b->border_color = GOLD;
    b->has_border   = 1;
    b->hovered      = 0;
}

int button_handle_event(Button *b, SDL_Event *e)
{
    if (e->type == SDL_MOUSEMOTION) {
        SDL_Point p = {e->motion.x, e->motion.y};
        b->hovered = SDL_PointInRect(&p, &b->rect);
    }
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        SDL_Point p = {e->button.x, e->button.y};
        if (SDL_PointInRect(&p, &b->rect)) return 1;
    }
    return 0;
}

void button_render(Button *b, SDL_Renderer *r, TTF_Font *font)
{
    SDL_Color bg = b->hovered ? b->hover_color : b->normal_color;
    draw_rounded_rect(r, &b->rect, 12, bg);

    if (b->has_border)
        draw_rounded_rect_border(r, &b->rect, 12, b->border_color, 2);

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, b->text, b->text_color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = {
            b->rect.x + (b->rect.w - surf->w) / 2,
            b->rect.y + (b->rect.h - surf->h) / 2,
            surf->w, surf->h
        };
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

/* ── TextInput ──────────────────────────────── */

static const SDL_Color TI_BG       = {50, 50, 70, 255};
static const SDL_Color TI_BORDER   = {100, 100, 120, 255};
static const SDL_Color TI_ACTIVE   = {70, 130, 180, 255};
static const SDL_Color TI_TEXT     = {240, 240, 240, 255};
static const SDL_Color TI_CURSOR   = {255, 255, 255, 255};
static const Uint32 CURSOR_BLINK_MS = 530;

void textinput_init(TextInput *t, int x, int y, int w, int h)
{
    t->rect = (SDL_Rect){x, y, w, h};
    memset(t->buf, 0, sizeof(t->buf));
    t->len            = 0;
    t->active         = 0;
    t->cursor_visible = 1;
    t->cursor_timer   = 0;
    t->bg_color       = TI_BG;
    t->border_color   = TI_BORDER;
    t->active_color   = TI_ACTIVE;
    t->text_color     = TI_TEXT;
}

void textinput_handle_event(TextInput *t, SDL_Event *e)
{
    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        SDL_Point p = {e->button.x, e->button.y};
        t->active = SDL_PointInRect(&p, &t->rect);
    }

    if (!t->active) return;

    if (e->type == SDL_KEYDOWN) {
        /* Ctrl+V paste */
        if (e->key.keysym.sym == SDLK_v
            && (e->key.keysym.mod & KMOD_CTRL)) {
            if (SDL_HasClipboardText()) {
                char *clip = SDL_GetClipboardText();
                if (clip) {
                    int clen = (int)strlen(clip);
                    if (t->len + clen > TEXTINPUT_MAX)
                        clen = TEXTINPUT_MAX - t->len;
                    if (clen > 0) {
                        memcpy(t->buf + t->len, clip, clen);
                        t->len += clen;
                        t->buf[t->len] = '\0';
                        t->cursor_visible = 1;
                        t->cursor_timer   = 0;
                    }
                    SDL_free(clip);
                }
            }
            return;
        }
        if (e->key.keysym.sym == SDLK_BACKSPACE && t->len > 0) {
            t->len--;
            while (t->len > 0 && (t->buf[t->len] & 0xC0) == 0x80)
                t->len--;
            t->buf[t->len] = '\0';
            t->cursor_visible = 1;
            t->cursor_timer   = 0;
        }
    }

    if (e->type == SDL_TEXTINPUT) {
        int inlen = (int)strlen(e->text.text);
        if (t->len + inlen <= TEXTINPUT_MAX) {
            memcpy(t->buf + t->len, e->text.text, inlen);
            t->len += inlen;
            t->buf[t->len] = '\0';
            t->cursor_visible = 1;
            t->cursor_timer   = 0;
        }
    }
}

void textinput_update(TextInput *t, Uint32 dt_ms)
{
    t->cursor_timer += dt_ms;
    if (t->cursor_timer >= CURSOR_BLINK_MS) {
        t->cursor_timer -= CURSOR_BLINK_MS;
        t->cursor_visible = !t->cursor_visible;
    }
}

void textinput_render(TextInput *t, SDL_Renderer *r, TTF_Font *font)
{
    SDL_Color border = t->active ? t->active_color : t->border_color;
    draw_rounded_rect(r, &(SDL_Rect){t->rect.x - 2, t->rect.y - 2,
                                      t->rect.w + 4, t->rect.h + 4}, 10, border);
    draw_rounded_rect(r, &t->rect, 8, t->bg_color);

    int clip_w = t->rect.w - 20;
    int text_w = 0;
    SDL_Surface *surf = NULL;
    SDL_Texture *tex = NULL;

    if (t->buf[0]) {
        surf = TTF_RenderUTF8_Blended(font, t->buf, t->text_color);
        if (surf) {
            tex = SDL_CreateTextureFromSurface(r, surf);
            if (tex) {
                text_w = surf->w;
                int src_x  = (text_w > clip_w) ? text_w - clip_w : 0;
                int draw_w = (text_w > clip_w) ? clip_w : text_w;
                SDL_Rect src_r = {src_x, 0, draw_w, surf->h};
                SDL_Rect dst_r = {t->rect.x + 10,
                                  t->rect.y + (t->rect.h - surf->h) / 2,
                                  draw_w, surf->h};
                SDL_RenderCopy(r, tex, &src_r, &dst_r);
            }
        }
    }

    if (t->active && t->cursor_visible) {
        int cursor_x = t->rect.x + 10 + text_w;
        if (text_w > clip_w) cursor_x = t->rect.x + 10 + clip_w;
        SDL_SetRenderDrawColor(r, TI_CURSOR.r, TI_CURSOR.g, TI_CURSOR.b, TI_CURSOR.a);
        SDL_RenderDrawLine(r, cursor_x, t->rect.y + 8, cursor_x, t->rect.y + t->rect.h - 8);
    }

    if (tex)  SDL_DestroyTexture(tex);
    if (surf) SDL_FreeSurface(surf);
}

/* ── Hint Overlay ───────────────────────────── */

void hint_overlay_init(HintOverlay *h)
{
    h->shown = 0;
    memset(h->message, 0, sizeof(h->message));
    button_init(&h->back_btn, 40, 40, 120, 42, "返回");
}

void hint_overlay_show(HintOverlay *h, const char *msg)
{
    h->shown = 1;
    snprintf(h->message, sizeof(h->message), "%s", msg);
}

void hint_overlay_hide(HintOverlay *h)
{
    h->shown = 0;
}

int hint_overlay_handle_event(HintOverlay *h, SDL_Event *e)
{
    if (!h->shown) return 0;
    if (button_handle_event(&h->back_btn, e)) {
        hint_overlay_hide(h);
        return 1;
    }
    return 0;
}

void hint_overlay_render(HintOverlay *h, SDL_Renderer *r, TTF_Font *font)
{
    if (!h->shown) return;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
    SDL_RenderFillRect(r, NULL);

    int bw = 500, bh = 300;
    SDL_Rect box = {(WINDOW_WIDTH - bw) / 2, (WINDOW_HEIGHT - bh) / 2, bw, bh};
    draw_rounded_rect(r, &box, 16, (SDL_Color){255, 255, 255, 255});

    SDL_Color msg_color = {30, 30, 30, 255};
    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, h->message, msg_color, bw - 60);
    if (surf) {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
        if (tex) {
            SDL_Rect dst = {
                box.x + (bw - surf->w) / 2,
                box.y + (bh - surf->h) / 2,
                surf->w, surf->h
            };
            SDL_RenderCopy(r, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
        }
        SDL_FreeSurface(surf);
    }

    button_render(&h->back_btn, r, font);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}
