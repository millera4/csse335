#ifndef VECTORH
#define VECTORH

typedef struct vector {
  double x;
  double y;
}vector;

vector plus( vector v1, vector v2){
  vector s;
  s.x=v1.x+v2.x;
  s.y=v1.y+v2.y;
  return s;
}

vector minus(vector v1, vector v2){
  vector s;
  s.x=v1.x-v2.x;
  s.y=v1.y-v2.y;
  return s;
}

vector mult(vector v1, double alpha){
  vector s;
  s.x=alpha*v1.x;
  s.y=alpha*v1.y;
  return s;
}

#endif
