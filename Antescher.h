#ifndef __ANTESCHER_H
#define __ANTESCHER_H

#define BOTTOM 2
#define TOP 1
#define SIDE 0

#include "SDL.h"
extern Uint8 Map[128][128];

extern bool LoadMap();
extern bool DoMovement(float *Pos, float *Vec, float *Size);

#endif
