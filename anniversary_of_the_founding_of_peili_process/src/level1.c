#include "level1.h"
#include "game.h"
#include <string.h>
#include <ctype.h>

/* ── Puzzle data ────────────────────────────── */

static const char *MORSE_CIPHER =
    "----. ---.. .---- ----- .---- ----- .---- .---- "
    "..... .---- ----- ----- .---- .---- "
    "..... .---- .---- ----- .---- "
    "...-- .---- .---- ..--- .---- ----- .---- ----- "
    "..... .---- ----- ---.. .---- ----- "
    "..... .---- ..--- ..--- .---- ----- "
    "....- .---- ----- "
    "..... .---- ..--- ..--- .---- .---- --... .---- ----- "
    ".---- .---- ----- ----. .---- .---- --... --... .---- .---- -----";

static const char *STEP1_ANSWER =
    "9810110510610511010311210110510810512210410512110112011710112111797110";

/* STEP1_HINT moved to hint overlay only — no longer shown inline */

static const char *STEP2_PROMPT = "所以美国信息交换标准代码是什么玩意？";

static const char *STEP2_ANSWER = "北京培黎职业学院";

static const char *HINT_STEP1 =
    "摩斯电码步骤提示：\n"
    "救救我！救救我！SOS！.-.!\n"
    "摩斯码中，每个字母用空格分隔，\n"
    "试着将摩斯码逐个翻译成数字吧！";

static const char *HINT_STEP2 =
    "ASCII码步骤提示：\n"
    "所以美国信息交换标准代码是什么玩意？\n"
    "把上一步的数字串每2-3位一组，\n"
    "查ASCII表转成字符！";

/* ── Debug helper ───────────────────────────── */

static int is_debug(const char *s)
{
    for (int i = 0; s[i]; i++)
        if (tolower((unsigned char)s[i]) != "debug"[i] && tolower((unsigned char)s[i]) != '\0')
            return 0;
    return strcasecmp(s, "debug") == 0;
}

/* ── Local rendering helpers ────────────────── */

static void l1_render_text_centered(SDL_Renderer *r, TTF_Font *font,
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

/* Render wrapped text, return the pixel height consumed */
static int l1_render_wrapped(SDL_Renderer *r, TTF_Font *font,
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

void level1_init(Level1State *s)
{
    s->step        = 1;
    s->error_count = 0;
    s->start_time  = SDL_GetTicks();
    s->completed   = 0;
}

/* ── Event ──────────────────────────────────── */

static void submit_answer(Level1State *s, Game *g)
{
    /* Debug cheat: "debug" skips current step */
    if (is_debug(g->input.buf)) {
        if (s->step == 1) {
            s->step = 2;
            s->error_count = 0;
            s->start_time  = SDL_GetTicks();
            textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
        } else {
            s->completed = 1;
            g->level1_completed = 1;
            save_game(g);
        }
        g->input.buf[0]       = '\0';
        g->input.len          = 0;
        g->input.cursor_visible = 1;
        g->input.cursor_timer   = 0;
        return;
    }

    const char *answer = (s->step == 1) ? STEP1_ANSWER : STEP2_ANSWER;

    if (strcmp(g->input.buf, answer) == 0) {
        if (s->step == 1) {
            s->step = 2;
            s->error_count = 0;
            s->start_time  = SDL_GetTicks();
            textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
        } else {
            s->completed = 1;
            g->level1_completed = 1;
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

void level1_handle_event(Level1State *s, Game *g, SDL_Event *e)
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

void level1_render(Level1State *s, Game *g)
{
    int cx = 50;
    int cw = WINDOW_WIDTH - 100;

    /* ── Title bar ── */
    l1_render_text_centered(g->renderer, g->font_large,
        "第一关：历史的回响", (SDL_Color){255, 215, 0, 255}, 8);

    if (s->completed) {
        l1_render_text_centered(g->renderer, g->font_large,
            "恭喜！第一关完成！", (SDL_Color){0, 255, 100, 255}, 200);
        l1_render_text_centered(g->renderer, g->font_medium,
            "答案：北京培黎职业学院", (SDL_Color){255, 215, 0, 255}, 270);
        l1_render_text_centered(g->renderer, g->font,
            "按 Enter 返回主菜单", (SDL_Color){200, 200, 200, 255}, 340);
        return;
    }

    /* ── Step indicator ── */
    int y = 70;
    const char *step_label = (s->step == 1)
        ? "步骤 1/2 — 摩斯电码" : "步骤 2/2 — ASCII码";
    l1_render_wrapped(g->renderer, g->font_medium,
        step_label, (SDL_Color){180, 180, 255, 255}, cx, y, cw);
    y += 38;

    if (s->step == 1) {
        /* ── Cipher block ── */
        y += 6;
        l1_render_wrapped(g->renderer, g->font,
            "密文：", (SDL_Color){255, 180, 180, 255}, cx, y, cw);
        y += 26;
        int h = l1_render_wrapped(g->renderer, g->font,
            MORSE_CIPHER, (SDL_Color){255, 255, 255, 255}, cx, y, cw);
        y += h + 14;

        /* ── Input prompt ── */
        l1_render_wrapped(g->renderer, g->font,
            "请输入摩斯码解码后的数字串：",
            (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    } else {
        /* ── Step 2 prompt ── */
        y += 6;
        int h = l1_render_wrapped(g->renderer, g->font_medium,
            STEP2_PROMPT, (SDL_Color){255, 255, 255, 255}, cx, y, cw);
        y += h + 16;

        l1_render_wrapped(g->renderer, g->font,
            "请输入解码后的中文：",
            (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    }

    /* ── Input + Submit (fixed near bottom) ── */
    textinput_render(&g->input, g->renderer, g->font);
    button_render(&g->btn_submit, g->renderer, g->font);
}

/* ── Hint text ──────────────────────────────── */

const char *level1_hint_text(Level1State *s)
{
    return (s->step == 1) ? HINT_STEP1 : HINT_STEP2;
}
