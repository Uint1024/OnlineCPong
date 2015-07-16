
#include <stdio.h>
#include "utils.h"
#include "world.h"
#include "server.h"

static IPaddress        server_ipaddress;
static TCPsocket        servsock = NULL;
static TCPsocket        clientsock = NULL;
static UDPsocket        udpsock = NULL;
static SDLNet_SocketSet socketset = NULL;
static UDPpacket        **packets = NULL;
static Uint16           listen_port;
static IPaddress        client_address;

//static Client clients[MAX_PLAYERS];

int Server_Init(int argc, char** argv)
{
    if(argc < 3){
        printf("Usage: name.exe --server listen_port");
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

    // The player is the server so it has id 0
    World_SetPlayerId(0);

    // Convert argv[2] to Uint16
    listen_port = atoi(argv[2]);

    // Put the server's IP and port in the IPaddress struct 
    // (in big endian format!)
    if(SDLNet_ResolveHost(&server_ipaddress, 
                NULL, listen_port) == -1){
        SDLNET_PRINT_ERROR(SDLNet_ResolveHost);
        return -1;
    }

    // Open server socket to listen to it
    servsock = SDLNet_TCP_Open(&server_ipaddress);
    if(!servsock){
        SDLNET_PRINT_ERROR(SDLNet_TCP_Open);
        return -1;
    }

    if(SDLNet_TCP_AddSocket(socketset, servsock) == -1){
        SDLNET_PRINT_ERROR(SDLNet_TCP_AddSocket);
        return -1;
    }

    // MUST open the UDP socket here and add it to socketset
    // It won't work if it's done after the first TCP connection
    // for some reason
    udpsock = SDLNet_UDP_Open(listen_port);
    if(!udpsock){
        SDLNET_PRINT_ERROR(SDLNet_UDP_Open);
        return -1;
    }

    if(SDLNet_UDP_AddSocket(socketset, udpsock) == -1){
        SDLNET_PRINT_ERROR(SDLNet_UDP_AddSocket);
        return -1;
    }

    return 0;
}

int Server_ReceiveNewClientTCPConnection()
{
    printf("Client connected.\n");
    // Create new socket for the client
    // (if there were more than 1 client we would need an array)
    clientsock = SDLNet_TCP_Accept(servsock);
    if(!clientsock){
        SDLNET_PRINT_ERROR(SDLNet_TCP_Accept);
        // Don't return -1, it's not an important error (?)
        //return -1;
    }

    // Add the socket listen to it
    if(SDLNet_TCP_AddSocket(socketset, clientsock) == -1){
        SDLNET_PRINT_ERROR(SDLNet_TCP_AddSocket);
        //return -1;
    }

    return 0;
}

int Server_ReceiveTCPData()
{
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

        // Change the port of client_address to the new one
        // we just received 
        client_address.port = *data;

        // Now that we know the client's address
        // and port we can bind it to the udp port
        // to be able to receive packets from it
        if(SDLNet_UDP_Bind(udpsock, 1, &client_address) == -1){
            SDLNET_PRINT_ERROR(SDLNet_UDP_Bind);
            fprintf(stderr, "Unable to bind client to UDP!\n");
        }
    }
    free(data);

    return 0;
}

int Server_ReceiveUDPData()
{
    int n = SDLNet_UDP_Recv(udpsock, packets[0]);
    Entity* r = (Entity*)packets[0]->data;
    World_GetPlayer(1)->position.x = r->position.x;
    World_GetPlayer(1)->position.y = r->position.y;
    if(n <= 0){
        printf("Received %i packets??\n", n);
    }

    return 0;
}

int Server_CheckSockets() 
{
    // Check sockets to see if there's data ready
    int numready = SDLNet_CheckSockets(socketset, 0);
    if (numready) {
        // A client connected through TCP!
        if (SDLNet_SocketReady(servsock)) {
            Server_ReceiveNewClientTCPConnection();
        }

        // Receiving data from client through TCP
        // The data can only be a port number!
        if(SDLNet_SocketReady(clientsock)){
            Server_ReceiveTCPData();
        }

        // Receive data through UDP
        if(SDLNet_SocketReady(udpsock)){
            Server_ReceiveUDPData();
        }
    }

    return 0;
}

int Server_SendDataToClients()
{
    if(clientsock != NULL){
        memcpy(packets[0]->data, World_GetThisPlayer(), sizeof(Entity));
        packets[0]->len = sizeof(Entity);
        if(SDLNet_UDP_Send(udpsock, 1, packets[0]) < 0){
            SDLNET_PRINT_ERROR(SDLNet_UDP_Send);
        }
    }

    return 0;
}

int Server_Run()
{
    Server_CheckSockets();
    Server_SendDataToClients();

    return 0;
}
