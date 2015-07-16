#include "entity.h"

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
 * Add the movement variable to the position variable
 */
void Entity_ApplyMovement(Entity* e){
  Vector_Addff(&e->position, &e->movement); 
}

