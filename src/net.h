#include <SDL_net.h>
#include <stdbool.h>
#ifndef NET_H
#define NET_H

#define MAX_PLAYERS 4

typedef struct Client {
    IPaddress   address;
    TCPsocket   clientsock;
} Client;

typedef struct NetConfig {
    IPaddress        server_ipaddress;
    TCPsocket        servsock;
    UDPsocket        udpsock;
    SDLNet_SocketSet socketset;
    UDPpacket        **packets;
    bool             is_server;
    Uint16           listen_port;
    Uint16           connect_port;
} NetConfig;

#if 0
void NetConfig_Constructor(NetConfig* c)
{
    c->servsock = NULL;
    c->udpsock = NULL;
    c->socketset = NULL;
    c->packets = NULL;
    c->is_server = false;

}
#endif

int Net_Init();

//static inline Net_GetConfig();

#endif
