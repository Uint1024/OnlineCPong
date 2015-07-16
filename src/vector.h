#ifndef VECTOR_H
#define VECTOR_H

typedef struct Vectorf {
  float x;
  float y;
} Vectorf;

typedef struct Vectori {
  int x;
  int y;
} Vectori;

void Vector_Addff(Vectorf* a, Vectorf* b);
void Vector_Addif(Vectori* a, Vectorf* b);
void Vector_Addfi(Vectorf* a, Vectori* b);
void Vector_Addii(Vectori* a, Vectori* b);
#endif
