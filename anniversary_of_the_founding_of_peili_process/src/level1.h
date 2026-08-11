#ifndef LEVEL1_H
#define LEVEL1_H

#include <SDL2/SDL.h>

typedef struct Game Game;

typedef struct {
    int     step;          /* 1 or 2 */
    int     error_count;
    Uint32  start_time;
    int     completed;     /* 1 when level finished */
} Level1State;

void level1_init(Level1State *s);
void level1_handle_event(Level1State *s, Game *g, SDL_Event *e);
void level1_render(Level1State *s, Game *g);
const char *level1_hint_text(Level1State *s);

#endif
