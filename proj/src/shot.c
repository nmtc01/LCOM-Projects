#include "shot.h"

bool able;

shot* new_shot(unsigned x, unsigned y, unsigned damage, unsigned hg, unsigned wd){
  shot* s = malloc(sizeof(*s));
  if(s == NULL)
    return NULL;
  s->x = x;
  s->y = y;
  s->speedX = 0;
  s->speedY = 0;
  s->width = wd;
  s->height = hg;
  s->damage = damage;
  s->active = true;
  return s;
}

int delete_shot(shot* s){
  if(s == NULL){
    return 1;
  }
  able = true;
  s->active = false;
  s = NULL;
  return 0;
}
