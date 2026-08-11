#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "ui.h"
#include "level1.h"
#include "level2.h"
#include "level3.h"

/* WINDOW_WIDTH / WINDOW_HEIGHT defined in ui.h */
#define WINDOW_TITLE  "培黎43周年校庆解密小游戏"

#define HINT_ERROR_THRESHOLD 3
#define HINT_TIME_THRESHOLD_MS (300 * 1000u)
#define SAVE_FILE  "save.dat"
#define XOR_KEY    0x5A

typedef enum {
    STATE_MENU,
    STATE_LEVEL_SELECT,
    STATE_LEVEL_1,
    STATE_LEVEL_2,
    STATE_LEVEL_3,
    STATE_HINT
} GameState;

typedef struct Game {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    GameState     current_state;
    int           running;

    /* Resources */
    TTF_Font     *font;        /* 22px standard */
    TTF_Font     *font_medium; /* 28px */
    TTF_Font     *font_large;  /* 40px titles */
    SDL_Texture  *bg_tex;
    SDL_Texture  *logo_tex;
    int           logo_w, logo_h;
    SDL_Texture  *qr_tex[4];
    int           qr_w[4], qr_h[4];

    /* Main menu buttons */
    Button        btn_start;
    Button        btn_select;
    Button        btn_quit;

    /* Level-select buttons */
    Button        btn_level[3];
    Button        btn_back_menu;

    /* In-level state */
    int           error_count;
    Uint32        level_time_ms;
    int           hint_visible;

    /* In-level UI */
    TextInput     input;
    Button        btn_hint;
    Button        btn_level_back;
    Button        btn_submit;

    /* Per-level state */
    Level1State   level1;
    Level2State   level2;
    Level3State   level3;

    /* Hint overlay */
    HintOverlay   hint;

    /* Persistent completion flags */
    int           level1_completed;
    int           level2_completed;
    int           level3_completed;
} Game;

int  game_init(Game *g);
void game_cleanup(Game *g);
void game_handle_event(Game *g, SDL_Event *e);
void game_update(Game *g, Uint32 dt_ms);
void game_render(Game *g);

void save_game(Game *g);
int  load_game(Game *g);

#endif
