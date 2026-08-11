#include "game.h"
#include <stdio.h>

static SDL_Texture *load_texture(Game *g, const char *path)
{
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        fprintf(stderr, "IMG_Load(%s) failed: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(g->renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
    return tex;
}

static TTF_Font *load_font(const char *path, int size)
{
    TTF_Font *f = TTF_OpenFont(path, size);
    if (!f) {
        fprintf(stderr, "TTF_OpenFont(%s, %d) failed: %s\n", path, size, TTF_GetError());
        return NULL;
    }
    return f;
}

static void enter_level(Game *g, GameState state)
{
    g->current_state = state;
    g->error_count   = 0;
    g->level_time_ms = 0;
    g->hint_visible  = 0;
    hint_overlay_hide(&g->hint);

    if (state == STATE_LEVEL_1) {
        level1_init(&g->level1);
        textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
    } else if (state == STATE_LEVEL_2) {
        level2_init(&g->level2);
        textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
    } else if (state == STATE_LEVEL_3) {
        level3_init(&g->level3);
        textinput_init(&g->input, WINDOW_WIDTH / 2 - 200, 510, 400, 40);
    } else {
        textinput_init(&g->input, WINDOW_WIDTH / 2 - 150, 400, 300, 40);
    }
}

int game_init(Game *g)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    g->window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!g->window) { fprintf(stderr, "CreateWindow: %s\n", SDL_GetError()); goto fail; }

    g->renderer = SDL_CreateRenderer(g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) { fprintf(stderr, "CreateRenderer: %s\n", SDL_GetError()); goto fail; }

    /* Fonts: simhei → simsun → msyh fallback */
    g->font = load_font("C:/Windows/Fonts/simhei.ttf", 22);
    if (!g->font) g->font = load_font("C:/Windows/Fonts/simsun.ttc", 22);
    if (!g->font) g->font = load_font("C:/Windows/Fonts/msyh.ttc", 22);
    g->font_medium = load_font("C:/Windows/Fonts/simhei.ttf", 28);
    if (!g->font_medium) g->font_medium = load_font("C:/Windows/Fonts/simsun.ttc", 28);
    if (!g->font_medium) g->font_medium = load_font("C:/Windows/Fonts/msyh.ttc", 28);
    g->font_large = load_font("C:/Windows/Fonts/simhei.ttf", 40);
    if (!g->font_large) g->font_large = load_font("C:/Windows/Fonts/simsun.ttc", 40);
    if (!g->font_large) g->font_large = load_font("C:/Windows/Fonts/msyh.ttc", 40);
    /* Size fallback: use smaller font if larger unavailable */
    if (!g->font_large)  g->font_large  = g->font_medium;
    if (!g->font_large)  g->font_large  = g->font;
    if (!g->font_medium) g->font_medium = g->font;
    if (!g->font) goto fail;

    /* Resources */
    g->bg_tex   = load_texture(g, "assets/bg.jpg");
    g->logo_tex = load_texture(g, "assets/logo.webp");
    if (g->logo_tex)
        SDL_QueryTexture(g->logo_tex, NULL, NULL, &g->logo_w, &g->logo_h);

    /* QR fragment images for level 3 */
    const char *qr_paths[4] = {
        "assets/qr1.png", "assets/qr2.png",
        "assets/qr3.png", "assets/qr4.png"
    };
    for (int i = 0; i < 4; i++) {
        g->qr_tex[i] = load_texture(g, qr_paths[i]);
        if (g->qr_tex[i])
            SDL_QueryTexture(g->qr_tex[i], NULL, NULL, &g->qr_w[i], &g->qr_h[i]);
    }

    /* ── Main menu buttons ── */
    int btn_w = 220, btn_h = 50;
    int btn_x = WINDOW_WIDTH / 2 - btn_w / 2;
    int btn_y0 = 380, btn_gap = 64;
    button_init_gold(&g->btn_start,  btn_x, btn_y0,            btn_w, btn_h, "开始游戏");
    button_init_gold(&g->btn_select, btn_x, btn_y0 + btn_gap,  btn_w, btn_h, "选关");
    button_init_gold(&g->btn_quit,   btn_x, btn_y0 + btn_gap*2,btn_w, btn_h, "退出游戏");

    /* ── Level-select buttons ── */
    int lv_btn_w = 320, lv_btn_h = 60, lv_gap = 80;
    int lv_x = WINDOW_WIDTH / 2 - lv_btn_w / 2;
    int lv_y0 = 200;
    button_init_gold(&g->btn_level[0], lv_x, lv_y0,           lv_btn_w, lv_btn_h, "第一关：历史的回响");
    button_init_gold(&g->btn_level[1], lv_x, lv_y0 + lv_gap,  lv_btn_w, lv_btn_h, "第二关：校史的迷雾");
    button_init_gold(&g->btn_level[2], lv_x, lv_y0 + lv_gap*2,lv_btn_w, lv_btn_h, "第三关：终极拼图");
    button_init_gold(&g->btn_back_menu, 30, 30, 140, 42, "返回主菜单");

    /* ── In-level UI ── */
    textinput_init(&g->input, WINDOW_WIDTH / 2 - 150, 400, 300, 40);
    button_init(&g->btn_hint, WINDOW_WIDTH - 130, WINDOW_HEIGHT - 60, 110, 40, "提示");
    button_init_gold(&g->btn_level_back, 20, 20, 140, 42, "< 主菜单");
    button_init_gold(&g->btn_submit, WINDOW_WIDTH / 2 + 210, 510, 80, 40, "提交");

    /* ── Hint overlay ── */
    hint_overlay_init(&g->hint);

    g->current_state     = STATE_MENU;
    g->running           = 1;
    g->error_count       = 0;
    g->level_time_ms     = 0;
    g->hint_visible      = 0;
    g->level1_completed  = 0;
    g->level2_completed  = 0;
    g->level3_completed  = 0;

    load_game(g);

    SDL_StartTextInput();
    return 0;

fail:
    game_cleanup(g);
    return -1;
}

void game_cleanup(Game *g)
{
    SDL_StopTextInput();
    if (g->logo_tex)    SDL_DestroyTexture(g->logo_tex);
    if (g->bg_tex)      SDL_DestroyTexture(g->bg_tex);
    for (int i = 0; i < 4; i++)
        if (g->qr_tex[i]) SDL_DestroyTexture(g->qr_tex[i]);
    if (g->font_large)  TTF_CloseFont(g->font_large);
    if (g->font_medium) TTF_CloseFont(g->font_medium);
    if (g->font)        TTF_CloseFont(g->font);
    if (g->renderer)    SDL_DestroyRenderer(g->renderer);
    if (g->window)      SDL_DestroyWindow(g->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

/* ── Save / Load (XOR encrypted) ───────────── */

void save_game(Game *g)
{
    unsigned char data[7];
    data[0] = 'P';
    data[1] = 'L';
    data[2] = 1; /* version */
    data[3] = (unsigned char)(g->level1_completed ? 1 : 0);
    data[4] = (unsigned char)(g->level2_completed ? 1 : 0);
    data[5] = (unsigned char)(g->level3_completed ? 1 : 0);
    data[6] = data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4] ^ data[5];

    for (int i = 0; i < 7; i++)
        data[i] ^= XOR_KEY;

    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) {
        fprintf(stderr, "save_game: cannot write %s\n", SAVE_FILE);
        return;
    }
    fwrite(data, 1, 7, f);
    fclose(f);
}

int load_game(Game *g)
{
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return -1; /* no save file — not an error */

    unsigned char data[7];
    if (fread(data, 1, 7, f) != 7) { fclose(f); return -1; }
    fclose(f);

    for (int i = 0; i < 7; i++)
        data[i] ^= XOR_KEY;

    if (data[0] != 'P' || data[1] != 'L' || data[2] != 1) return -1;
    unsigned char check = data[0] ^ data[1] ^ data[2]
                        ^ data[3] ^ data[4] ^ data[5];
    if (check != data[6]) return -1;

    g->level1_completed = data[3];
    g->level2_completed = data[4];
    g->level3_completed = data[5];
    return 0;
}

/* ── Helpers ────────────────────────────────── */

static int in_level(Game *g)
{
    return g->current_state == STATE_LEVEL_1
        || g->current_state == STATE_LEVEL_2
        || g->current_state == STATE_LEVEL_3;
}

static const char *hint_text_for_state(Game *g)
{
    if (g->current_state == STATE_LEVEL_1)
        return level1_hint_text(&g->level1);
    if (g->current_state == STATE_LEVEL_2)
        return level2_hint_text(&g->level2);
    if (g->current_state == STATE_LEVEL_3)
        return level3_hint_text(&g->level3);
    return "暂无提示";
}

static void render_bg_with_dim(Game *g, Uint8 alpha)
{
    if (g->bg_tex) {
        SDL_RenderCopy(g->renderer, g->bg_tex, NULL, NULL);
        SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g->renderer, 0, 0, 0, alpha);
        SDL_RenderFillRect(g->renderer, NULL);
        SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_NONE);
    } else {
        SDL_SetRenderDrawColor(g->renderer, 30, 30, 46, 255);
        SDL_RenderClear(g->renderer);
    }
}

static void render_text_centered(SDL_Renderer *r, TTF_Font *font,
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

/* ── Event handling ─────────────────────────── */

void game_handle_event(Game *g, SDL_Event *e)
{
    if (e->type == SDL_QUIT) {
        g->running = 0;
        return;
    }

    /* Hint overlay intercepts all events when shown */
    if (g->hint.shown) {
        hint_overlay_handle_event(&g->hint, e);
        return;
    }

    switch (g->current_state) {
    case STATE_MENU:
        if (button_handle_event(&g->btn_start, e))
            enter_level(g, STATE_LEVEL_1);
        else if (button_handle_event(&g->btn_select, e))
            g->current_state = STATE_LEVEL_SELECT;
        else if (button_handle_event(&g->btn_quit, e))
            g->running = 0;
        break;

    case STATE_LEVEL_SELECT:
        if (button_handle_event(&g->btn_back_menu, e)) {
            g->current_state = STATE_MENU;
            break;
        }
        for (int i = 0; i < 3; i++) {
            if (button_handle_event(&g->btn_level[i], e)) {
                enter_level(g, STATE_LEVEL_1 + i);
                break;
            }
        }
        break;

    case STATE_LEVEL_1:
        /* Level complete: Enter or back returns to menu */
        if (g->level1.completed) {
            if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN) {
                g->current_state = STATE_MENU;
                g->error_count   = 0;
                g->level_time_ms = 0;
                g->hint_visible  = 0;
            }
            break;
        }
        if (button_handle_event(&g->btn_level_back, e)) {
            g->current_state = STATE_MENU;
            g->error_count   = 0;
            g->level_time_ms = 0;
            g->hint_visible  = 0;
            break;
        }
        if (g->hint_visible && button_handle_event(&g->btn_hint, e)) {
            hint_overlay_show(&g->hint, hint_text_for_state(g));
            break;
        }
        level1_handle_event(&g->level1, g, e);
        break;

    case STATE_LEVEL_2:
        if (g->level2.completed) {
            if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN) {
                g->current_state = STATE_MENU;
                g->error_count   = 0;
                g->level_time_ms = 0;
                g->hint_visible  = 0;
            }
            break;
        }
        if (button_handle_event(&g->btn_level_back, e)) {
            g->current_state = STATE_MENU;
            g->error_count   = 0;
            g->level_time_ms = 0;
            g->hint_visible  = 0;
            break;
        }
        if (g->hint_visible && button_handle_event(&g->btn_hint, e)) {
            hint_overlay_show(&g->hint, hint_text_for_state(g));
            break;
        }
        level2_handle_event(&g->level2, g, e);
        break;

    case STATE_LEVEL_3:
        if (g->level3.completed) {
            if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_RETURN) {
                g->current_state = STATE_MENU;
                g->error_count   = 0;
                g->level_time_ms = 0;
                g->hint_visible  = 0;
            }
            break;
        }
        if (button_handle_event(&g->btn_level_back, e)) {
            g->current_state = STATE_MENU;
            g->error_count   = 0;
            g->level_time_ms = 0;
            g->hint_visible  = 0;
            break;
        }
        if (g->hint_visible && button_handle_event(&g->btn_hint, e)) {
            hint_overlay_show(&g->hint, hint_text_for_state(g));
            break;
        }
        level3_handle_event(&g->level3, g, e);
        break;

    default:
        break;
    }
}

/* ── Update ─────────────────────────────────── */

void game_update(Game *g, Uint32 dt_ms)
{
    textinput_update(&g->input, dt_ms);

    if (in_level(g)) {
        g->level_time_ms += dt_ms;
        if (!g->hint_visible) {
            if (g->error_count >= HINT_ERROR_THRESHOLD
             || g->level_time_ms >= HINT_TIME_THRESHOLD_MS) {
                g->hint_visible = 1;
            }
        }
    }
}

/* ── Rendering ──────────────────────────────── */

static void render_menu(Game *g)
{
    render_bg_with_dim(g, 128);

    if (g->logo_tex) {
        int target_w = WINDOW_WIDTH / 5;
        float scale = (float)target_w / g->logo_w;
        int dw = target_w;
        int dh = (int)(g->logo_h * scale);
        SDL_Rect dst = {(WINDOW_WIDTH - dw) / 2, 40, dw, dh};
        SDL_RenderCopy(g->renderer, g->logo_tex, NULL, &dst);
    } else {
        SDL_SetRenderDrawColor(g->renderer, 255, 105, 180, 255);
        SDL_Rect placeholder = {(WINDOW_WIDTH - 120) / 2, 40, 120, 120};
        SDL_RenderFillRect(g->renderer, &placeholder);
    }

    render_text_centered(g->renderer, g->font_large,
        "喜迎培黎43周年校庆", (SDL_Color){255, 0, 0, 255}, 200);
    render_text_centered(g->renderer, g->font_medium,
        "解密小游戏", (SDL_Color){255, 215, 0, 255}, 260);

    button_render(&g->btn_start,  g->renderer, g->font);
    button_render(&g->btn_select, g->renderer, g->font);
    button_render(&g->btn_quit,   g->renderer, g->font);
}

static void render_level_select(Game *g)
{
    render_bg_with_dim(g, 140);

    render_text_centered(g->renderer, g->font_large,
        "请选择关卡", (SDL_Color){255, 255, 255, 255}, 80);

    for (int i = 0; i < 3; i++) {
        button_render(&g->btn_level[i], g->renderer, g->font_medium);
        int done = (i == 0) ? g->level1_completed
                 : (i == 1) ? g->level2_completed
                 : g->level3_completed;
        if (done) {
            render_text_centered(g->renderer, g->font,
                "[已通关]", (SDL_Color){0, 255, 100, 255},
                g->btn_level[i].rect.y + g->btn_level[i].rect.h + 2);
        }
    }

    button_render(&g->btn_back_menu, g->renderer, g->font);
}

static void render_level(Game *g)
{
    render_bg_with_dim(g, 140);

    button_render(&g->btn_level_back, g->renderer, g->font);

    if (g->current_state == STATE_LEVEL_1) {
        level1_render(&g->level1, g);
    } else if (g->current_state == STATE_LEVEL_2) {
        level2_render(&g->level2, g);
    } else {
        level3_render(&g->level3, g);
    }

    if (g->hint_visible)
        button_render(&g->btn_hint, g->renderer, g->font);
}

void game_render(Game *g)
{
    switch (g->current_state) {
    case STATE_MENU:
        render_menu(g);
        break;
    case STATE_LEVEL_SELECT:
        render_level_select(g);
        break;
    case STATE_LEVEL_1:
    case STATE_LEVEL_2:
    case STATE_LEVEL_3:
        render_level(g);
        break;
    default:
        SDL_SetRenderDrawColor(g->renderer, 30, 30, 46, 255);
        SDL_RenderClear(g->renderer);
        break;
    }

    if (g->hint.shown)
        hint_overlay_render(&g->hint, g->renderer, g->font);

    SDL_RenderPresent(g->renderer);
}

/* ── Main ───────────────────────────────────── */

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game game = {0};
    if (game_init(&game) != 0) return 1;

    Uint32 last_tick = SDL_GetTicks();

    while (game.running) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            game_handle_event(&game, &e);

        Uint32 now = SDL_GetTicks();
        Uint32 dt  = now - last_tick;
        last_tick  = now;

        game_update(&game, dt);
        game_render(&game);
    }

    game_cleanup(&game);
    return 0;
}
