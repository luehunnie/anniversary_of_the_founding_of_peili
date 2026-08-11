#ifndef LEVEL3_H
#define LEVEL3_H

#include <SDL2/SDL.h>

typedef struct Game Game;

typedef struct {
    int     step;
    int     error_count;
    Uint32  start_time;
    int     completed;
} Level3State;

void level3_init(Level3State *s);
void level3_handle_event(Level3State *s, Game *g, SDL_Event *e);
void level3_render(Level3State *s, Game *g);
const char *level3_hint_text(Level3State *s);

#endif
