#include "level2.h"
#include "game.h"
#include <string.h>
#include <ctype.h>

/* ── Puzzle data ────────────────────────────── */

static const char *CIPHER =
    "NjZUNktfUnBpdl9waXZfTGlfQV9MaUxfS19SfGFfampfTHFPX0xpQV9xXzJq";

/* Answers pre-computed: Base64 decode → fence decode → Base64 decode → Chinese */
static const char *STEP1_ANSWER =
    "66T6K_Rpiv_piv_Li_A_LiL_K_R|a_jj_LqO_LiA_q_2j";
static const char *STEP2_ANSWER =
    "6_6KT_6RK|_aR_pjijv__LpqiOv__LLiiA__Aq__L2ijL";
static const char *STEP3_ANSWER = "手脑并用，创造分析";

/* Hint overlay texts per step (only shown via hint button) */
static const char *HINT_STEP1 =
    "Base64步骤提示：\n"
    "贝斯64是什么贝斯？\n"
    "试着用Base64解码这段密文吧！";
static const char *HINT_STEP2 =
    "栅栏密码步骤提示：\n"
    "我的世界里栅栏有多高？2格！\n"
    "用2栏栅栏密码还原上一步的结果。";
static const char *HINT_STEP3 =
    "第二次Base64步骤提示：\n"
    "不对，贝斯是什么乐器？\n"
    "把栅栏还原的结果再做一次Base64解码！";

/* Input prompts */
static const char *PROMPT_STEP1 = "请输入 Base64 解码后的字符串：";
static const char *PROMPT_STEP2 = "请输入栅栏密码（2栏）还原后的字符串：";
static const char *PROMPT_STEP3 = "请输入最终解码结果（中文口号）：";

/* Step labels */
static const char *LABEL_STEP1 = "步骤 1/3 — Base64 解码";
static const char *LABEL_STEP2 = "步骤 2/3 — 栅栏密码（2栏）";
static const char *LABEL_STEP3 = "步骤 3/3 — 二次 Base64 解码";

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

static void l2_render_text_centered(SDL_Renderer *r, TTF_Font *font,
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

static int l2_render_wrapped(SDL_Renderer *r, TTF_Font *font,
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

void level2_init(Level2State *s)
{
    s->step        = 1;
    s->error_count = 0;
    s->start_time  = SDL_GetTicks();
    s->completed   = 0;
}

/* ── Event ──────────────────────────────────── */

static void submit_answer(Level2State *s, Game *g)
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
            g->level2_completed = 1;
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
            g->level2_completed = 1;
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

void level2_handle_event(Level2State *s, Game *g, SDL_Event *e)
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

void level2_render(Level2State *s, Game *g)
{
    int cx = 50;
    int cw = WINDOW_WIDTH - 100;

    /* Title */
    l2_render_text_centered(g->renderer, g->font_large,
        "第二关：校史的迷雾", (SDL_Color){255, 215, 0, 255}, 8);

    if (s->completed) {
        l2_render_text_centered(g->renderer, g->font_large,
            "恭喜！第二关完成！", (SDL_Color){0, 255, 100, 255}, 200);
        l2_render_text_centered(g->renderer, g->font_medium,
            "答案：手脑并用，创造分析", (SDL_Color){255, 215, 0, 255}, 270);
        l2_render_text_centered(g->renderer, g->font,
            "按 Enter 返回主菜单", (SDL_Color){200, 200, 200, 255}, 340);
        return;
    }

    /* Step label */
    int y = 70;
    const char *label = (s->step == 1) ? LABEL_STEP1
                      : (s->step == 2) ? LABEL_STEP2
                      : LABEL_STEP3;
    l2_render_wrapped(g->renderer, g->font_medium,
        label, (SDL_Color){180, 180, 255, 255}, cx, y, cw);
    y += 38;

    if (s->step == 1) {
        /* Cipher text */
        y += 6;
        l2_render_wrapped(g->renderer, g->font,
            "密文：", (SDL_Color){255, 180, 180, 255}, cx, y, cw);
        y += 26;
        int h = l2_render_wrapped(g->renderer, g->font,
            CIPHER, (SDL_Color){255, 255, 255, 255}, cx, y, cw);
        y += h + 14;

        /* Input prompt */
        l2_render_wrapped(g->renderer, g->font,
            PROMPT_STEP1, (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    } else if (s->step == 2) {
        l2_render_wrapped(g->renderer, g->font,
            PROMPT_STEP2, (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    } else {
        l2_render_wrapped(g->renderer, g->font,
            PROMPT_STEP3, (SDL_Color){220, 220, 220, 255}, cx, y, cw);
    }

    /* Input + Submit (fixed at bottom) */
    textinput_render(&g->input, g->renderer, g->font);
    button_render(&g->btn_submit, g->renderer, g->font);
}

/* ── Hint text ──────────────────────────────── */

const char *level2_hint_text(Level2State *s)
{
    switch (s->step) {
    case 1:  return HINT_STEP1;
    case 2:  return HINT_STEP2;
    default: return HINT_STEP3;
    }
}
