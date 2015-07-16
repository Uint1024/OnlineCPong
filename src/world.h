#include "entity.h"
#ifndef WORLD_H
#define WORLD_H


int World_Init();

Entity* World_GetPlayer(int id);
Entity* World_GetBall();
void World_Update();
Entity* World_GetThisPlayer();
void World_SetPlayerId(int id);
int World_GetPlayerId();
#endif
