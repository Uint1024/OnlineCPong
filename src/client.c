#include <SDL_net.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.h"
#include "client.h"
#include "world.h"

#define DEFAULT_UDP_PORT 18999

static IPaddress        server_ipaddress;
static TCPsocket        servsock = NULL;
static UDPsocket        udpsock = NULL;
static SDLNet_SocketSet socketset = NULL;
static UDPpacket        **packets = NULL;
static Uint16           listen_port;
static Uint16           connect_port;

int Client_Init(int argc, char** argv)
{
    if(argc < 4){
        printf("Usage: name.exe --client ip connect_port\n");
        return -1;
    }

    // allocate sockets in a socket set ; it will be checked for incoming data
    socketset = SDLNet_AllocSocketSet(6);
    if(!socketset){
        SDLNET_PRINT_ERROR(SDLNet_AllocSocketSet);
        return -1;
    }

    // Alloc 4 packets of size Entity (because we'll only ever send this size!)
    packets = SDLNet_AllocPacketV(4, sizeof(Entity));
    if(!packets){
        SDLNET_PRINT_ERROR(SDLNet_AllocPacketV);
        return -1;
    }

    // Client is always id 1 (server is id 0)
    World_SetPlayerId(1);

    // Port used to connect to the server
    // It needs to corresponds to the server's listen port (duh)
    connect_port = (Uint16)atoi(argv[3]);

    printf("Launched the game as client. Will try to connect to %s:%s.\n", 
            argv[2],argv[3]);

    // Put the server's address and port in the IPaddress struct
    if(SDLNet_ResolveHost(&server_ipaddress, argv[2], connect_port) == -1){
        SDLNET_PRINT_ERROR(SDLNet_ResolveHost);
        return -1;
    }

    // Connect to the server through TCP
    servsock = SDLNet_TCP_Open(&server_ipaddress);
    if(!servsock){
        SDLNET_PRINT_ERROR(SDLNet_TCP_Open);
        return -1;
    }

    // Try a bunch of UDP ports because some may be in use
    listen_port = DEFAULT_UDP_PORT;
    while(udpsock == NULL && listen_port < DEFAULT_UDP_PORT + 100){
        listen_port += (Uint16)1;
        udpsock = SDLNet_UDP_Open(listen_port);
        printf("Opening port on %i\n", listen_port);
    }

    if(!udpsock){
        SDLNET_PRINT_ERROR(SDLNet_UDP_Open);
        return -1;
    }
        
    // Binding the UDP port to an address means that we'll be 
    // able to receive data from this address
    // (if it's not bound then the data is rejected!)
    if(SDLNet_UDP_Bind(udpsock, 1, &server_ipaddress) == -1){
        SDLNET_PRINT_ERROR(SDLNet_UDP_Bind);
        return -1;
    }

    // Add socket to the set
    if(SDLNet_UDP_AddSocket(socketset, udpsock) == -1){
        SDLNET_PRINT_ERROR(SDLNet_UDP_AddSocket);
        return -1;
    }

    /* 
     * Send the listen port to the server (in Big Endian format)
     * so that he knows where to send UDP packets!
     */
    Uint16 udplistenport = SDL_SwapBE16(listen_port);
    int sent_hello = SDLNet_TCP_Send(servsock, &udplistenport, sizeof(Uint16));

    if(sent_hello <= 0){
        SDLNET_PRINT_ERROR(SDLNet_TCP_Send);
        fprintf(stderr, "Was unable to send port number to the server.");
    }

    return 0;
}

void Client_ReceiveFromServerUDP(){
    int n = SDLNet_UDP_Recv(udpsock, packets[0]);
    if (n < 0) {
        printf("Received %i packets??\n", n);
    }
    Entity* r = (Entity*)packets[0]->data;
    World_GetPlayer(0)->position.x = r->position.x;
    World_GetPlayer(0)->position.y = r->position.y;
}

void Client_SendToServerUDP(){
    memcpy(packets[0]->data, World_GetThisPlayer(), sizeof(Entity));
    packets[0]->len = sizeof(Entity);
    packets[0]->address = server_ipaddress;
    int sent = SDLNet_UDP_Send(udpsock, 1, packets[0]);
    if(sent == 0){
        SDLNET_PRINT_ERROR(SDLNet_UDP_Send);
        fprintf(stderr, "Couldn't send data to server.");
    }
}

int Client_Run()
{
    int numready = SDLNet_CheckSockets(socketset, 0);

    if(numready){
        if(SDLNet_SocketReady(udpsock)){
            Client_ReceiveFromServerUDP();
        }
    }
    Client_SendToServerUDP();

    return 0;
}
