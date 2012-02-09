#ifndef __CUBEDRAWING_H
#define __CUBEDRAWING_H

#include "ebgf.h"

extern void DrawInvertedCube();
extern void DrawCube(Uint8 LFlags = 0, Uint8 RFlags = 0, Uint8 TFlags = 0, Uint8 BFlags = 0, Uint8 SFlags = 0);
extern void DrawCubeShadowVolume(float *LightPos, float *cubepos, float xl = 1.0f, float yl = 1.0f, float zl = 1.0f);
extern void DrawSphereShadowVolume(float *LightPos, float *cubepos, float xl = 1.0f, float yl = 1.0f, float zl = 1.0f);


#endif
