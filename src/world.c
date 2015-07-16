#include <stddef.h>
#include "entity.h"

// Player 0 is the server, 1 is the client
static Entity* players[2] = {NULL};
static Entity* ball = {NULL};
static int player_id = 0;

int World_Init()
{
    // Creating players
    // TODO: maybe the server should do it...
    players[0] = Paddle_New(50,50);
    players[1] = Paddle_New(300,300);

    ball = Ball_New(150,150);
}

Entity* World_GetPlayer(int id)
{
    return players[id];
}

Entity* World_GetBall()
{
    return ball;
}

void World_Update()
{
    // Update entities
    for(int i = 0 ; i < 2 ; ++i){
        Entity_ApplyMovement(players[i]);
        Entity_StopMoving(players[i]);
    }
}

void World_SetPlayerId(int id)
{
    player_id = id;
}

Entity* World_GetThisPlayer()
{
    return players[player_id];
}


int World_GetPlayerId()
{
    return player_id;
}
