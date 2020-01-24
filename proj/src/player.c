#include "player.h"

player *newPlayer(unsigned x, unsigned y, unsigned hg, unsigned wd) {
  player *p = malloc(sizeof(*p));
  if (p == NULL)
    return NULL;
  p->height = hg;
  p->width = wd;
  p->x = x;
  p->y = y;
  p->speedX = 0;
  p->speedY = 0;
  return p;
}

void deletePlayer(player *p) {
  if (p == NULL) {
    return;
  }
  free(p);
}
