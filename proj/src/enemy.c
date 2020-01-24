#include "enemy.h"

enemy* new_enemy(unsigned x, unsigned y, unsigned health, unsigned wde, unsigned hge, char path){
  enemy* e = malloc(sizeof(*e));
  if(e == NULL){
    return NULL;
  }
  e->x = x;
  e->y = y;
  e->og_x = x;
  e->og_y = y;

  e->speedX = -8;
  e->speedY = 8;
  e->health = health;
  e->width = wde;
  e->height = hge;

  e->path = path;

  e->active = true;
  return e;
}

void delete_enemy(enemy* e){
  if(e == NULL){
    return;
  }
  e->active = false;
}

enemy* load_enemy(unsigned x, unsigned y, unsigned og_x, unsigned og_y, unsigned height, unsigned width, int speedX, int speedY, unsigned health, char path, bool active){
  enemy* e = malloc(sizeof(*e));
  if(e == NULL){
    return NULL;
  }
  e->x = x;
  e->y = y;
  e->og_x = og_x;
  e->og_y = og_y;
  e->speedX = speedX;
  e->speedY = speedY;
  e->health = health;
  e->width = width;
  e->height = height;
  e->path = path;
  e->active = active;

  return e;
}
