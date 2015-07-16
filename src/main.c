/*
 * A very simple online 2-players pong game
 */

#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>

#include "entity.h"
#include "vector.h"
#include "renderer.h"
#include "utils.h"
#include "world.h"
#include "input.h"
#include "server.h"
#include "client.h"


// Print a big-endian uint32 integer in ip format
void PrintIpAddressFromBEUint32(Uint32 address){
    int byte1 = address>>24&0xFF;
    int byte2 = address>>16&0xFF;
    int byte3 = address>>8&0xFF;
    int byte4 = address&0xFF;
    printf("%i.%i.%i.%i",byte4,byte3,byte2,byte1); 
}


int InitSDL(){
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        SDL_PRINT_ERROR(SDL_Init);
        return -1;
    }

    if(SDLNet_Init() < 0){
        SDLNET_PRINT_ERROR(SDLNet_Init);
        SDL_Quit();
        return -1;
    }

    return 0;
}

int Init_Everything(int argc, char** argv){
    if(InitSDL() == -1){
        return -1;
    }

    if(Renderer_Init() == -1){
        return -1;
    }

    World_Init();

    // If the program is run as server
    if(strcmp(argv[1], "--server") == 0 ||
            strcmp(argv[1], "-s") == 0){
        if(Server_Init(argc, argv) == -1){
            fprintf(stderr, "InitServer error.");
            return -1;
        }
    }

    // If the program is run as client
    if(strcmp(argv[1], "--client") == 0 ||
            strcmp(argv[1], "-c") == 0){
        if(Client_Init(argc, argv) == -1){
            fprintf(stderr, "InitClient error.");
            return -1;
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    if(argc < 3){
        printf("Usage: \nname.exe --client ip connect_port listen_port"
                "\nor"
                "\nname.exe --server listen_port");
        return -1;
    }

    if(Init_Everything(argc, argv) == -1){
        return -1;
    }

    int keep_going = 1;
    while(keep_going){
        // is server
        if(World_GetPlayerId() == 0){
            Server_Run();
        } else { // is client
            Client_Run();
        }

        // Returns -1 when pressing Escape
        keep_going = Input_PollEvents();
        World_Update();
        Renderer_Render();
    }

    // Bye!
    Renderer_Destroy();

    // Calm down, compiler!
    return 0;
}
