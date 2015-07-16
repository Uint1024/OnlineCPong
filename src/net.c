#include "net.h"
#if 0
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
#endif

static Client clients[MAX_PLAYERS];

int Net_Init(){
}


