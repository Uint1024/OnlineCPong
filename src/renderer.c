#include <SDL.h>
#include "renderer.h"
#include "utils.h"
#include "world.h"
#include "renderer.h"

static SDL_Window* pWindow = NULL;
static SDL_Renderer* pRenderer = NULL;

int Renderer_Init()
{
    pWindow = SDL_CreateWindow("Online game test", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN);
    if(!pWindow){
        SDL_PRINT_ERROR(SDL_CreateWindow);
        return -1;
    }
    pRenderer = SDL_CreateRenderer(pWindow, -1, 
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(!pRenderer){
        SDL_PRINT_ERROR(SDL_CreateRenderer);
        return -1;
    }
}

int Renderer_Render()
{
    SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
    SDL_RenderClear(pRenderer);

    SDL_Rect player_rect;
    SDL_SetRenderDrawColor(pRenderer, 50, 255, 0, 255);
    for(int i = 0 ; i < 2; ++i){
        player_rect.x = (int)World_GetPlayer(i)->position.x;
        player_rect.y = (int)World_GetPlayer(i)->position.y;
        player_rect.w = (int)World_GetPlayer(i)->size.x;
        player_rect.h = (int)World_GetPlayer(i)->size.y;

        SDL_RenderFillRect(pRenderer, &player_rect);
    }

    SDL_RenderPresent(pRenderer);
}

int Renderer_Destroy() 
{
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
}
