/*
 * A very simple online 2-players pong game
 */

#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>

#define RF_WINDOW_WIDTH 600
#define RF_WINDOW_HEIGHT 400
#define SDLNET_PRINT_ERROR(function_name) fprintf(stderr, #function_name " error: %s", SDLNet_GetError())
#define SDL_PRINT_ERROR(function_name) fprintf(stderr, #function_name " error: %s", SDL_GetError())

//TODO: separate game in multiple files...

typedef struct Vectorf {
  float x;
  float y;
} Vectorf;

typedef struct Vectori {
  int x;
  int y;
} Vectori;


typedef struct Entity {
  Vectorf position;
  Vectori size;
  Vectorf movement;
} Entity;

/* 
 * Constructors for the Paddles and Balls
 * Only difference between the two is the size
 */
Entity* Paddle_New(float x, float y){
  Entity* ret = malloc(sizeof(Entity));

  ret->position   = (Vectorf){.x = x, .y = y};
  ret->size       = (Vectori){30, 100};
  ret->movement   = (Vectorf){0.0f, 0.0f};

  return ret;
}

Entity* Ball_New(float x, float y){
  Entity* ret     = malloc(sizeof(Entity));

  ret->position   = (Vectorf){x, y};
  ret->size       = (Vectori){30, 30};
  ret->movement   = (Vectorf){0.0f, 0.0f};

  return ret;
}


void Entity_StopMoving(Entity* e){
  e->movement.x = 0.0f;
  e->movement.y = 0.0f;
}

/*
 * Put sum of 2 float vectors in first vector
 */
void Vector_Addff(Vectorf* a, Vectorf* b){
  a->x += b->x;
  a->y += b->y;
}

/*
 * Add the movement variable to the position variable
 */
void Entity_ApplyMovement(Entity* e){
  Vector_Addff(&e->position, &e->movement); 
}

// Player 0 is the server, 1 is the client
static Entity* players[2] = {NULL};
static Entity* ball = {NULL};

// SDL stuff
static SDL_Event e;
static SDL_Window* pWindow = NULL;
static SDL_Renderer* pRenderer = NULL;

// Used by the server to listen to its TCP port,
// and by the client to connect to it
static IPaddress server_ipaddress;

// Like server_ipaddress, this socket is the server's local socket,
// but the client uses it to connect to the server
static TCPsocket servsock = NULL;

// The server listens to this socket after the first client connection
static TCPsocket clientsock = NULL;

static UDPsocket udpsock = NULL;

static UDPpacket **packets = NULL;
static SDLNet_SocketSet socketset = NULL;

static int player_id = 0;
static bool is_server = false;
static Uint16 listen_port;
static Uint16 connect_port;

// The server defines this variable after receiving the client's port number
// (he doesn't know the client's listen port at first)
static IPaddress client_address;

// Print a big-endian uint32 integer in ip format
void PrintIpAddressFromBEUint32(Uint32 address){
    int byte1 = address>>24&0xFF;
    int byte2 = address>>16&0xFF;
    int byte3 = address>>8&0xFF;
    int byte4 = address&0xFF;
    printf("%i.%i.%i.%i",byte4,byte3,byte2,byte1); 
}

static int err = 0;
int main(int argc, char** argv)
{
    if(argc < 3){
        printf("Usage: \nname.exe --client ip connect_port listen_port"
                "\nor"
                "\nname.exe --server listen_port");
        return 1;
    }

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        SDL_PRINT_ERROR(SDL_Init);
        return 1;
    }

    if(SDLNet_Init() < 0){
        SDLNET_PRINT_ERROR(SDLNet_Init);
        SDL_Quit();
        return 1;
    }

    // Use SDLNet_CheckSockets(socketset, delay) to know if data is ready
    // (allocate MORE than needed.)
    socketset = SDLNet_AllocSocketSet(6);
    if(!socketset){
        SDLNET_PRINT_ERROR(SDLNet_AllocSocketSet);
        return 1;
    }

    // Alloc 4 packets of size Entity (because we'll only ever send this size!)
    packets = SDLNet_AllocPacketV(4, sizeof(Entity));
    if(!packets){
        SDLNET_PRINT_ERROR(SDLNet_AllocPacketV);
        return 1;
    }


    // If the program is run as server
    if(strcmp(argv[1], "--server") == 0){
        if(argc < 3){
            printf("Usage: name.exe --server listen_port");
            return 1;
        }

        // The player is the server so it has id 0
        player_id = 0;
        is_server = true;

        // Convert argv[2] to Uint16
        listen_port = atoi(argv[2]);

        // Put the server's IP and port in the IPaddress struct 
        // (in big endian format!)
        if(SDLNet_ResolveHost(&server_ipaddress, NULL, listen_port) == -1){
            SDLNET_PRINT_ERROR(SDLNet_ResolveHost);
            return 1;
        }

        // Open server socket to listen to it
        servsock = SDLNet_TCP_Open(&server_ipaddress);
        if(!servsock){
            SDLNET_PRINT_ERROR(SDLNet_TCP_Open);
            return 1;
        }

        if(SDLNet_TCP_AddSocket(socketset, servsock) == -1){
            SDLNET_PRINT_ERROR(SDLNet_TCP_AddSocket);
            return 1;
        }

        // MUST open the UDP socket here and add it to socketset
        // It won't work if it's done after the first TCP connection
        // for some reason
        udpsock = SDLNet_UDP_Open(listen_port);
        if(!udpsock){
            SDLNET_PRINT_ERROR(SDLNet_UDP_Open);
            return 1;
        }

        if(SDLNet_UDP_AddSocket(socketset, udpsock) == -1){
            SDLNET_PRINT_ERROR(SDLNet_UDP_AddSocket);
        }
    }

    // Starting the program as client
    if(strcmp(argv[1], "--client") == 0){
        // Client needs to configure an extra port (might need to change that)
        if(argc < 5){
            printf("Usage: name.exe --client ip connect_ip listen_ip");
            return 1;
        }

        player_id = 1;

        // Port used to connect to the server
        // It needs to corresponds to the server's listen port
        connect_port = atoi(argv[3]);

        // The listen port will be sent to the server so he doesn't need
        // to know it beforehan
        listen_port = atoi(argv[4]);
        printf("Launched the game as client. Will try to connect to %s:%s.\n", 
                argv[2],argv[3]);

        // Same as server side, we put the server's info in the IPaddress struct
        if(SDLNet_ResolveHost(&server_ipaddress, argv[2], connect_port) == -1){
            SDLNET_PRINT_ERROR(SDLNet_ResolveHost);
            return 1;
        }

        servsock = SDLNet_TCP_Open(&server_ipaddress);
        if(!servsock){
            SDLNET_PRINT_ERROR(SDLNet_TCP_Open);
            return 1;
        }

        udpsock = SDLNet_UDP_Open(listen_port);
        if(!udpsock){
            SDLNET_PRINT_ERROR(SDLNet_UDP_Open);
            return 1;
        }

        // Binding the UDP port to an address means that we'll be 
        // able to receive data from this address
        // (if it's not bound then the data is rejected!)
        if(SDLNet_UDP_Bind(udpsock, 1, &server_ipaddress) == -1){
            SDLNET_PRINT_ERROR(SDLNet_UDP_Bind);
            return 1;
        }

        if (SDLNet_UDP_AddSocket(socketset, udpsock) == -1){
            SDLNET_PRINT_ERROR(SDLNet_UDP_AddSocket);
            return 1;
        }

        /* 
         * Send the listen port to the server
         * so that he knows where to send UDP packets!
         */
        Uint16 udplistenport = listen_port;
        int sent_hello = SDLNet_TCP_Send(servsock, &udplistenport, sizeof(Uint16));
        printf("Sent %i bytes.\n", sent_hello);
    }

    /* Basic SDL stuff */
    pWindow = SDL_CreateWindow("Online game test", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, RF_WINDOW_WIDTH, RF_WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN);
    if(!pWindow){
        SDL_PRINT_ERROR(SDL_CreateWindow);
    }
    pRenderer = SDL_CreateRenderer(pWindow, -1, 
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(!pRenderer){
        SDL_PRINT_ERROR(SDL_CreateRenderer);
    }

    // Creating players
    // TODO: maybe the server should do it...
    players[0] = Paddle_New(50,50);
    players[1] = Paddle_New(300,300);

    for(;;){
        if(is_server){
            // Check sockets to see if there's data ready
            int numready = SDLNet_CheckSockets(socketset, 0);
            if(numready){
                // A client connected through TCP!
                if(SDLNet_SocketReady(servsock)){
                    printf("Client connected.\n");
                    // Create new socket for the client
                    // (if there were more than 1 client we would need an array)
                    clientsock = SDLNet_TCP_Accept(servsock);
                    if(!clientsock){
                        SDLNET_PRINT_ERROR(SDLNet_TCP_Accept);
                    }

                    // Add the socket listen to it
                    if(SDLNet_TCP_AddSocket(socketset, clientsock) == -1){
                        SDLNET_PRINT_ERROR(SDLNet_TCP_AddSocket);
                    }
                    
                }

                // Receiving data from client through TCP
                if(SDLNet_SocketReady(clientsock)){
                    printf("Received something on TCP socket!\n");
                    // We know that the client only send its port through TCP
                    // so the data is only 2 bytes
                    Uint16* data = malloc(sizeof(Uint16));

                    if(SDLNet_TCP_Recv(clientsock, data, sizeof(Uint16)) <= 0){
                        printf("Error during receive, did the client disconnect?\n");
                        SDLNet_TCP_DelSocket(socketset, clientsock);
                        SDLNet_TCP_Close(clientsock);
                        clientsock = NULL;
                    }
                    else {
                        printf("Received %i.\n", *data);
                        // Fill the client_address struct
                        client_address = *SDLNet_TCP_GetPeerAddress(clientsock);
                        printf("Client IP is : ");
                        PrintIpAddressFromBEUint32(client_address.host);

                        // Change the port of client_address to the new one
                        // we just received 
                        // (we need to translate it to big endian first)
                        client_address.port = SDL_SwapBE16(*data);

                        // Now that we know the client's address
                        // and port we can bind it to the udp port
                        // to be able to receive packets from it
                        int numBound = SDLNet_UDP_Bind(udpsock, 1, &client_address);
                    }
                    free(data);
                }
                // Receive data through UDP
                if(SDLNet_SocketReady(udpsock)){
                    int n = SDLNet_UDP_Recv(udpsock, packets[0]);
                    Entity* r = (Entity*)packets[0]->data;
                    players[1]->position.x = r->position.x;
                    players[1]->position.y = r->position.y;
                    if(n <= 0){
                        printf("Received %i packets??\n", n);
                    }
                }
            }

            // Send data to client
            if(clientsock != NULL){
                memcpy(packets[0]->data, players[player_id], sizeof(Entity));
                packets[0]->len = sizeof(Entity);
                int numsent = SDLNet_UDP_Send(udpsock, 1, packets[0]);
            }
        }
        else{ //is client
            int numready = SDLNet_CheckSockets(socketset, 0);

            if(numready){
                // Receiving data from server through TCP
                // (this never happens)
#if 0
                if(SDLNet_SocketReady(servsock)){
                    Entity* data = malloc(sizeof(Entity)); 

                    if(SDLNet_TCP_Recv(servsock, data, sizeof(Entity)) <= 0){
                        printf("Error during receive, did the client disconnect?");
                        SDLNet_TCP_DelSocket(socketset, servsock);
                        SDLNet_TCP_Close(servsock);
                        servsock = NULL;
                    }
                    else {
                    }
                    free(data);
                }
#endif
                if(SDLNet_SocketReady(udpsock)){
                    int n = SDLNet_UDP_Recv(udpsock, packets[0]);
                    if(n < 0){
                        printf("Received %i packets??\n", n);
                    }
                    Entity* r = (Entity*)packets[0]->data;
                    players[0]->position.x = r->position.x;
                    players[0]->position.y = r->position.y;
                }
            }

            // Send data to server
            memcpy(packets[0]->data, players[player_id], sizeof(Entity));
            packets[0]->len = sizeof(Entity);
            packets[0]->address = server_ipaddress;
            int sent = SDLNet_UDP_Send(udpsock, 1, packets[0]);
            if(sent == 0){
                printf("Error during send: %s.\n", SDLNet_GetError());
            }
        }

        // Poll events
        while(SDL_PollEvent(&e)){
            int scancode = e.key.keysym.scancode;
            switch(e.type){
            case SDL_KEYDOWN:
                if(SDL_SCANCODE_S == scancode)
                    players[player_id]->movement.y += 5;
                if(SDL_SCANCODE_W == scancode)
                    players[player_id]->movement.y -= 5;
                if(SDL_SCANCODE_ESCAPE== scancode)
                    return 0;
            break;

            }
        }

        // Update entities
        for(int i = 0 ; i < 2 ; ++i){
            Entity_ApplyMovement(players[i]);
            Entity_StopMoving(players[i]);
        }

        // Render
        SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
        SDL_RenderClear(pRenderer);

        SDL_Rect player_rect;
        SDL_SetRenderDrawColor(pRenderer, 50, 255, 0, 255);
        for(int i = 0 ; i < 2; ++i){
            player_rect.x = (int)players[i]->position.x;
            player_rect.y = (int)players[i]->position.y;
            player_rect.w = (int)players[i]->size.x;
            player_rect.h = (int)players[i]->size.y;

            SDL_RenderFillRect(pRenderer, &player_rect);
        }

        SDL_RenderPresent(pRenderer);
    }

    // Bye!
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);

}
