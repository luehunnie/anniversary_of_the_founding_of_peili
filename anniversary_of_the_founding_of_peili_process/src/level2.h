#ifndef LEVEL2_H
#define LEVEL2_H

#include <SDL2/SDL.h>

typedef struct Game Game;

typedef struct {
    int     step;
    int     error_count;
    Uint32  start_time;
    int     completed;
} Level2State;

void level2_init(Level2State *s);
void level2_handle_event(Level2State *s, Game *g, SDL_Event *e);
void level2_render(Level2State *s, Game *g);
const char *level2_hint_text(Level2State *s);

#endif
