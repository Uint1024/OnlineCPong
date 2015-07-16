#ifndef CLIENT_H
#define CLIENT_H

int Client_Init(int argc, char** argv);
int Client_Run();
void Client_SendToServerUDP();
void Client_ReceiveFromServerUDP();
#endif
