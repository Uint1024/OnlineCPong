#include <SDL_net.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.h"
#include "client.h"
#include "world.h"

// Used by the server to listen to its TCP port,
// and by the client to connect to it
static IPaddress        server_ipaddress;

// Like server_ipaddress, this socket is the server's local socket,
// but the client uses it to connect to the server
static TCPsocket        servsock = NULL;

// The server listens to this socket after the first client connection
static UDPsocket        udpsock = NULL;
static SDLNet_SocketSet socketset = NULL;
static UDPpacket        **packets = NULL;
static bool             is_server = false;
static Uint16           listen_port;
static Uint16           connect_port;

// The server defines this variable after receiving the client's port number
// (he doesn't know the client's listen port at first)
static IPaddress        client_address;

int Client_Init(int argc, char** argv)
{
    // Client needs to configure an extra port (might need to change that)
    if(argc < 5){
        printf("Usage: name.exe --client ip connect_ip listen_ip\n");
        return -1;
    }

    // Use SDLNet_CheckSockets(socketset, delay) to know if data is ready
    // (allocate MORE than needed.)
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

    World_SetPlayerId(1);

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
        return -1;
    }

    servsock = SDLNet_TCP_Open(&server_ipaddress);
    if(!servsock){
        SDLNET_PRINT_ERROR(SDLNet_TCP_Open);
        return -1;
    }

    udpsock = SDLNet_UDP_Open(listen_port);
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

    if(SDLNet_UDP_AddSocket(socketset, udpsock) == -1){
        SDLNET_PRINT_ERROR(SDLNet_UDP_AddSocket);
        return -1;
    }

    /* 
     * Send the listen port to the server
     * so that he knows where to send UDP packets!
     */
    Uint16 udplistenport = listen_port;
    int sent_hello = SDLNet_TCP_Send(servsock, &udplistenport, sizeof(Uint16));
    printf("Sent %i bytes.\n", sent_hello);
}

int Client_Run()
{
    int numready = SDLNet_CheckSockets(socketset, 0);

    if(numready){
        if(SDLNet_SocketReady(udpsock)){
            int n = SDLNet_UDP_Recv(udpsock, packets[0]);
            if (n < 0) {
                printf("Received %i packets??\n", n);
            }
            Entity* r = (Entity*)packets[0]->data;
            World_GetPlayer(0)->position.x = r->position.x;
            World_GetPlayer(0)->position.y = r->position.y;
        }
    }

    // Send data to server
    memcpy(packets[0]->data, World_GetThisPlayer(), sizeof(Entity));
    packets[0]->len = sizeof(Entity);
    packets[0]->address = server_ipaddress;
    int sent = SDLNet_UDP_Send(udpsock, 1, packets[0]);
    if(sent == 0){
        printf("Error during send: %s.\n", SDLNet_GetError());
    }
}
