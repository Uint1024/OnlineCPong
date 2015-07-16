#include <SDL.h>
#include "world.h"
#include "input.h"

static SDL_Event e;
int Input_PollEvents()
{
    while (SDL_PollEvent(&e)) {
        int scancode = e.key.keysym.scancode;
        switch (e.type) {
            case SDL_KEYDOWN:
                if(SDL_SCANCODE_S == scancode)
                    World_GetThisPlayer()->movement.y += 5;
                if(SDL_SCANCODE_W == scancode)
                    World_GetThisPlayer()->movement.y -= 5;
                if(SDL_SCANCODE_ESCAPE == scancode){
                    return 0;
                }
                break;

        }
    }
    return 1;
}
