#include "net.h"

#ifndef SERVER_H
#define SERVER_H
int Server_Init(int argc, char** argv);
int Server_Run();
int Server_SendDataToClients();
#endif
