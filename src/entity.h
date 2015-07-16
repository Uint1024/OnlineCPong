#include "vector.h"

#ifndef ENTITY_H
#define ENTITY_H

typedef struct Entity {
  Vectorf position;
  Vectori size;
  Vectorf movement;
} Entity;

Entity* Paddle_New(float x, float y);
Entity* Ball_New(float x, float y);
void Entity_StopMoving(Entity* e);
void Entity_ApplyMovement(Entity* e);

#endif
