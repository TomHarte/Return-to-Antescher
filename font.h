#ifndef __FONT_H
#define __FONT_H

#include "SDL_opengl.h"

enum TextAlign{EBGFTA_CENTRE, EBGFTA_LEFT, EBGFTA_RIGHT};
#define EBGFTA_TOP		EBGFTA_LEFT
#define EBGFTA_BOTTOM	EBGFTA_RIGHT

extern void EBGF_PrintText(TextAlign vmode, TextAlign hmode, char *str, ...);
extern float EBGF_TextWidth(char *str, ...);
extern float EBGF_TextHeight();
extern void setuptext();

#endif
