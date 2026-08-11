#include "level3.h"
#include "game.h"
#include <string.h>
#include <ctype.h>

/* ── Puzzle data ────────────────────────────── */

static const char *VIGENERE_CIPHER =
    "HGY NOPW TKK BM 1983-2026, DKDK NWE FA ZY, TKK BM IJQRM SYRM";

static const char *STEP1_ANSWER = "2026PL43ZN";
static const char *STEP2_ANSWER =
    "SVZ AZEX GVZ CZ 1983-2026, OZEX YLF SL OZ, GVZ CZ TYREX HZEX";
static const char *STEP3_ANSWER = "北京培黎1983到2026，星火不熄，培黎常青";

/* Hint overlay texts per step */
static const char *HINT_STEP1 =
    "二维码步骤提示：\n"
    "二维码兄弟你行不行啊，怎么裂开了？";
static const char *HINT_STEP2 =
    "维吉尼亚步骤提示：\n"
    "维吉，维吉，一定是维吉尔！";
static const char *HINT_STEP3 =
    "凯撒密码步骤提示：\n"
    "让地中海短暂当了洗脚盆的帝国。";

/* Input prompts */
static const char *PROMPT_STEP1 = "请输入拼合后二维码扫描出的内容（维吉尼亚密钥）：";
static const char *PROMPT_STEP2 = "请输入维吉尼亚解密后的字符串：";
static const char *PROMPT_STEP3 = "请输入最终解密结果（中文）：";

/* Step labels */
static const char *LABEL_STEP1 = "步骤 1/3 — 二维码拼合与扫描";
static const char *LABEL_STEP2 = "步骤 2/3 — 维吉尼亚密码";
static const char *LABEL_STEP3 = "步骤 3/3 — 凯撒密码 → 中文";

/* ── Debug helper ───────────────────────────── */

static int is_debug(const char *s)
{
    return strcasecmp(s, "debug") == 0;
}

/* ── Case-insensitive compare ───────────────── */

static int ci_strcmp(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 1;
        a++; b++;
    }
    return *a != *b;
}

/* ── Local rendering helpers ────────────────── */

static void l3_render_text_centered(SDL_Renderer *r, TTF_Font *font,
    const char *text, SDL_Color color, int y)
{
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = {(WINDOW_WIDTH - surf->w) / 2, y, surf->w, surf->h};
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

static int l3_render_wrapped(SDL_Renderer *r, TTF_Font *font,
    const char *text, SDL_Color color, int x, int y, int max_w)
{
    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, text, color, max_w);
    if (!surf) return 0;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    int h = surf->h;
    SDL_FreeSurface(surf);
    return h;
}

/* ── Init ───────────────────────────────────── */

void level3_init(Level3State *s)
{
    s->step        = 1;
    s->error_count = 0;
    s->start_time  = SDL_GetTicks();
    s->completed   = 0;
}

/* ── Event ──────────────────────────────────── */

static void submit_answer(Level3State *s, Game *g)
{
    /* Debug cheat */
    if (is_debug(g->input.buf)) {
        if (s->step < 3) {
            s->step++;
            s->error_count = 0;
            s->start_time  = SDL_GetTicks();
            textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
        } else {
            s->completed = 1;
            g->level3_completed = 1;
            save_game(g);
        }
        g->input.buf[0]       = '\0';
        g->input.len          = 0;
        g->input.cursor_visible = 1;
        g->input.cursor_timer   = 0;
        return;
    }

    int correct = 0;
    switch (s->step) {
    case 1: correct = (ci_strcmp(g->input.buf, STEP1_ANSWER) == 0); break;
    case 2: correct = (ci_strcmp(g->input.buf, STEP2_ANSWER) == 0); break;
    case 3: correct = (strcmp(g->input.buf, STEP3_ANSWER) == 0);  break;
    }

    if (correct) {
        if (s->step < 3) {
            s->step++;
            s->error_count = 0;
            s->start_time  = SDL_GetTicks();
            textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
        } else {
            s->completed = 1;
            g->level3_completed = 1;
            save_game(g);
        }
    } else {
        s->error_count++;
        g->error_count = s->error_count;
    }

    g->input.buf[0]       = '\0';
    g->input.len          = 0;
    g->input.cursor_visible = 1;
    g->input.cursor_timer   = 0;
}

void level3_handle_event(Level3State *s, Game *g, SDL_Event *e)
{
    if (button_handle_event(&g->btn_submit, e)) {
        submit_answer(s, g);
        return;
    }

    textinput_handle_event(&g->input, e);

    if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN
        && g->input.active) {
        submit_answer(s, g);
    }
}

/* ── Render ─────────────────────────────────── */

void level3_render(Level3State *s, Game *g)
{
    int cx = 50;
    int cw = WINDOW_WIDTH - 100;

    /* Title */
    l3_render_text_centered(g->renderer, g->font_large,
        "第三关：终极拼图", (SDL_Color){255, 215, 0, 255}, 8);

    if (s->completed) {
        l3_render_text_centered(g->renderer, g->font_large,
            "恭喜！第三关完成！", (SDL_Color){0, 255, 100, 255}, 200);
        l3_render_text_centered(g->renderer, g->font_medium,
            "答案：北京培黎1983到2026，星火不熄，培黎常青",
            (SDL_Color){255, 215, 0, 255}, 270);
        l3_render_text_centered(g->renderer, g->font,
            "按 Enter 返回主菜单", (SDL_Color){200, 200, 200, 255}, 340);
        return;
    }

    /* Step label */
    int y = 70;
    const char *label = (s->step == 1) ? LABEL_STEP1
                      : (s->step == 2) ? LABEL_STEP2
                      : LABEL_STEP3;
    l3_render_wrapped(g->renderer, g->font_medium,
        label, (SDL_Color){180, 180, 255, 255}, cx, y, cw);
    y += 38;

    if (s->step == 1) {
        /* ── 2×2 QR fragment grid ── */
        int cell = 150;
        int gap  = 16;
        int grid_w = 2 * cell + gap;
        int grid_h = 2 * cell + gap;
        int grid_x = (WINDOW_WIDTH - grid_w) / 2;
        int grid_y = y + 10;

        for (int i = 0; i < 4; i++) {
            int row = i / 2, col = i % 2;
            int tx = grid_x + col * (cell + gap);
            int ty = grid_y + row * (cell + gap);

            if (!g->qr_tex[i]) {
                SDL_SetRenderDrawColor(g->renderer, 255, 105, 180, 255);
                SDL_RenderFillRect(g->renderer, &(SDL_Rect){tx, ty, cell, cell});
                continue;
            }

            float sw = (float)cell / g->qr_w[i];
            float sh = (float)cell / g->qr_h[i];
            float sc = sw < sh ? sw : sh;
            int dw = (int)(g->qr_w[i] * sc);
            int dh = (int)(g->qr_h[i] * sc);

            SDL_Rect dst = {
                tx + (cell - dw) / 2,
                ty + (cell - dh) / 2,
                dw, dh
            };
            SDL_RenderCopy(g->renderer, g->qr_tex[i], NULL, &dst);
        }

        y = grid_y + grid_h + 16;

        /* Input prompt */
        l3_render_wrapped(g->renderer, g->font,
            PROMPT_STEP1, (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    } else if (s->step == 2) {
        /* Cipher text */
        y += 6;
        l3_render_wrapped(g->renderer, g->font,
            "密文：", (SDL_Color){255, 180, 180, 255}, cx, y, cw);
        y += 26;
        int h = l3_render_wrapped(g->renderer, g->font,
            VIGENERE_CIPHER, (SDL_Color){255, 255, 255, 255}, cx, y, cw);
        y += h + 14;

        /* Input prompt */
        l3_render_wrapped(g->renderer, g->font,
            PROMPT_STEP2, (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    } else {
        /* Step 3 prompt */
        l3_render_wrapped(g->renderer, g->font,
            PROMPT_STEP3, (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    }

    /* Input + Submit (fixed at bottom) */
    textinput_render(&g->input, g->renderer, g->font);
    button_render(&g->btn_submit, g->renderer, g->font);
}

/* ── Hint text ──────────────────────────────── */

const char *level3_hint_text(Level3State *s)
{
    switch (s->step) {
    case 1:  return HINT_STEP1;
    case 2:  return HINT_STEP2;
    default: return HINT_STEP3;
    }
}
