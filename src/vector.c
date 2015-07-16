#include "vector.h"

void Vector_Addff(Vectorf* a, Vectorf* b){
  a->x += b->x;
  a->y += b->y;
}

void Vector_Addif(Vectori* a, Vectorf* b){
  a->x += b->x;
  a->y += b->y;
}

void Vector_Addfi(Vectorf* a, Vectori* b){
  a->x += b->x;
  a->y += b->y;
}

void Vector_Addii(Vectori* a, Vectori* b){
  a->x += b->x;
  a->y += b->y;
}
