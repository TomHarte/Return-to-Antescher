/*
 *  ebgf.h
 *  Event Based Game Framework
 *
 *  Created by Thomas Harte on 15/10/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __EBGF_H
#define __EBGF_H

#include "SDL_opengl.h"
#include "SDL.h"
#include "font.h"

/* the message structure */
enum EBGF_MessageType {EBGF_PROGSTART, EBGF_PROGEND, EBGF_SETUPDISPLAY, EBGF_UPDATE, EBGF_DRAW, EBGF_ENTER, EBGF_EXIT, EBGF_SDLMESSAGE};
struct EBGF_Message
{
	EBGF_MessageType Type;
	union
	{
		struct{
			Uint32 Age;
		} Update;
		struct{
			Uint32 Age;
		} Draw;
		struct{
			Uint32 Age;
			SDL_Event *Event;
		} SDLMessage;
	} Data;
};

/* base class for each screen */
class CGameScreen
{
	public:
		virtual void Message(EBGF_Message &Message) {};
};

/* base class for game */ 
class CGame: public CGameScreen
{
	public:
		virtual CGameScreen *GetFirstScreen() {return this;}
};

/* implement this one yourself so that EBGF knows where to start */
extern CGame *GetGame();

/* functions for dictating various things */
extern void EBGF_SetLogicFrequency(int BeatsPerSecond);
extern void EBGF_PushScreen(CGameScreen *scr);

/* functions for getting the horizontal and vertical field of view - these return a number in degrees! */
extern float EBGF_GetHFOV();
extern float EBGF_GetVFOV();
extern float flrand();
extern int EBGF_GetMouseMickeys(int &x, int &y);
extern void EBGF_ClearMouseMickeys();

enum EBGF_StackCommand {EBGF_DESCEND, EBGF_ASCEND, EBGF_REMOVE, EBGF_RESTART};
extern void EBGF_DoStackCommand(EBGF_StackCommand cmd);

#endif
