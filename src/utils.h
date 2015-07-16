#include <stdint.h>
#include <stdio.h>

#define DEFAULT_WINDOW_WIDTH 600
#define DEFAULT_WINDOW_HEIGHT 400
#define SDLNET_PRINT_ERROR(function_name) fprintf(stderr, #function_name " error: %s", SDLNet_GetError())
#define SDL_PRINT_ERROR(function_name) fprintf(stderr, #function_name " error: %s", SDL_GetError())
