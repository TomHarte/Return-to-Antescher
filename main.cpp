#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "Platform.h"
#include "Gameplay.h" //TODO: don't do this, don't load/save settings here! 

#include "SDL.h"
#include "SDL_mixer.h"
#include "ebgf.h"
#include "font.h"

static SDL_Surface *gScreen;
int MickeyX, MickeyY, MouseButtons;
int EBGF_GetMouseMickeys(int &X, int &Y)
{
	X = MickeyX; MickeyX = 0;
	Y = MickeyY; MickeyY = 0;
	return MouseButtons;
}

void EBGF_ClearMouseMickeys()
{
	MickeyX = MickeyY = 0;
}

float HFOV, VFOV;

float EBGF_GetHFOV()
{
	return HFOV;
}

float EBGF_GetVFOV()
{
	return VFOV;
}

float flrand()
{
	return (float)rand() / RAND_MAX;
}

bool SetupGFX(int w, int h, bool fs, bool ms, bool hr)
{
	/* if fs, find new w, h */
	if(fs)
	{
					SDL_Rect **Modes = SDL_ListModes(NULL, SDL_OPENGL | SDL_FULLSCREEN);
					SDL_Rect *Res;
					if(hr)
						Res = Modes[0];
					else
					{
						/* find first mode with height = 480, or take first higher */
						int index = 0;
						while(Modes[index] && Modes[index]->h > 480)
							index++;
						if(!Modes[index] || Modes[index]->h < 480) index--;
						Res = Modes[index];
					}
					
		w = Res->w;
		h = Res->h;
	}

	/* store fields of view */
	VFOV = 45.0f;
	HFOV = ((float)w / (float)h) * 45.0f;
	
	/* don't accept HFOV > 180 */
	if(HFOV > 180)
	{
		h = (int)((float)w / (180.0f / 45.0f));
		HFOV = ((float)w / (float)h) * 45.0f;
	}
	
	if(ms)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}

	if(gScreen = SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (fs ? SDL_FULLSCREEN : SDL_RESIZABLE)))
	{	
		/* default projection is a 45 degree vertical FOV, positive z = further into screen, perspective, z range 1 -> 1000 */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, (float)w / (float)h, 0.1, 400);
		glScalef(1.0f, 1.0f, -1.0f);

		/* default modelview is identity */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		/* enable depth test */
		glEnable(GL_DEPTH_TEST);

		/* enable reverse face removal */
		glEnable(GL_CULL_FACE);

		/* set "ordinary" alpha */
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	}

	return gScreen != NULL ? true : false;
}

CGameScreen *GameScreenStack[32];
int GameStackReadPointer, GameStackWritePointer;

void EBGF_PushScreen(CGameScreen *scr)
{
	GameScreenStack[GameStackWritePointer] = scr;
	GameStackWritePointer++;
}

void EBGF_SyncWritePointer()
{
	if(GameStackWritePointer <= GameStackReadPointer) return;
	while(1)
	{
		GameStackWritePointer--;
		if(GameStackWritePointer > GameStackReadPointer)
			delete GameScreenStack[GameStackWritePointer];
		else
		{
			GameStackWritePointer++;
			return;
		}
	}
}

EBGF_StackCommand PendingCommands[32];
int CommandPointer;
void EBGF_DoStackCommand(EBGF_StackCommand cmd)
{
	PendingCommands[CommandPointer] = cmd;
	CommandPointer++;
}

int BeatsPerSecond;
void EBGF_SetLogicFrequency(int bps)
{
	BeatsPerSecond = bps;
}

#define SendSetupDisplay()	\
	msg.Type = EBGF_SETUPDISPLAY;\
	if(GameScreenStack[GameStackReadPointer] != AttachedGame) GameScreenStack[GameStackReadPointer]->Message(msg);\
	AttachedGame->Message(msg);

int main(int argc, char *argv[])
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		exit(1);
	setuptext();
	srand(time(NULL));

	/* double buffer is non-negotiable */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/* depth size of 32, stencil of 8, multisamples 4 desirable */
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	bool HighRes = false;
	bool MultiSample = false;
	bool GrabMouse = false;

	/* vsync? Hell yeah! */
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	/* accelerated or nothing */
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// display defaults
	int Width = 640, Height = 480;
	bool FullScr = false;
	float Gamma = 1.0f;

	/* load whatever settings we can */
	char SaveName[2048];
	FILE *SaveHandle;
	sprintf(SaveName, "%santescher.cfg", GetSavePath());
	if(SaveHandle = fopen(SaveName, "rb"))
	{
		/* yes, that's right Ñ someone might score 4 billion points */
		HighScore = fgetc(SaveHandle);
		HighScore |= fgetc(SaveHandle) << 8;
		HighScore |= fgetc(SaveHandle) << 16;
		HighScore |= fgetc(SaveHandle) << 24;

		/* get highres, multisample & fullscr settings */
		Uint8 Flags = fgetc(SaveHandle);
		HighRes = (Flags&1) ? true : false;
		MultiSample = (Flags&2) ? true : false;
		GrabMouse = FullScr = (Flags&4) ? true : false;
		SDL_ShowCursor(GrabMouse ? SDL_DISABLE : SDL_ENABLE);

		/* get window size */
		Width = fgetc(SaveHandle);
		Width |= fgetc(SaveHandle) << 8;
		Width |= fgetc(SaveHandle) << 16;
		Width |= fgetc(SaveHandle) << 24;
		Height = fgetc(SaveHandle);
		Height |= fgetc(SaveHandle) << 8;
		Height |= fgetc(SaveHandle) << 16;
		Height |= fgetc(SaveHandle) << 24;

		/* get gamma */
		Uint32 FixGamma;
		FixGamma = fgetc(SaveHandle);
		FixGamma |= fgetc(SaveHandle) << 8;
		FixGamma |= fgetc(SaveHandle) << 16;
		FixGamma |= fgetc(SaveHandle) << 24;
		Gamma = (float)FixGamma / 65536.0f;

		fclose(SaveHandle);
	}
	
	// open audio
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 8192);

	// get CGame derivative, tell it to initialise
	CGame *AttachedGame = GetGame();
	EBGF_Message msg;
	msg.Type = EBGF_PROGSTART;
	AttachedGame->Message(msg);

	// get first screen
	GameStackReadPointer = GameStackWritePointer = 0;
	CommandPointer = 0;
	EBGF_PushScreen(AttachedGame->GetFirstScreen());

	// create display
	if(!SetupGFX(Width, Height, FullScr, MultiSample, HighRes))
	{
		printf("Unable to open graphics mode!\n");
		exit(1);
	}
	
	// allow game and scren to setup special graphics requirements
	SendSetupDisplay();

	// send initial ENTER message
	msg.Type = EBGF_ENTER;
	GameScreenStack[GameStackReadPointer]->Message(msg);

	// run event loop
	bool Quit = false;
	Uint32 OldTime = SDL_GetTicks(), SpareMS = 0;
	Uint32 ScreenAge = 0;
	BeatsPerSecond = 100; // default logic rate: 100 bps
	
	while(!Quit)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			bool FixRes = false, OldGrabMouse = GrabMouse;
			switch(event.type)
			{
				default: break;
				case SDL_QUIT:
					Quit = true;
				break;
				case SDL_VIDEORESIZE:
					Width = event.resize.w;
					Height = event.resize.h;
					FixRes = true;

					/* send setup display messages */
					SendSetupDisplay();
				break;
				case SDL_MOUSEBUTTONDOWN:
					GrabMouse = true;
				break;
				case SDL_ACTIVEEVENT:
					if(!event.active.gain && event.active.state && !FullScr)
						GrabMouse = false;
				break;
				case SDL_KEYDOWN:
					/* some special key downs: ESC returns to the top screen or quits if the program is already there */
					switch(event.key.keysym.sym)
					{
						default: break;
						case SDLK_ESCAPE:
							if(!GameStackReadPointer) Quit = true;
							else
							{
								msg.Type = EBGF_EXIT;
								GameScreenStack[GameStackReadPointer]->Message(msg);

								GameStackReadPointer = 0; ScreenAge = 0;
								EBGF_SyncWritePointer();

								msg.Type = EBGF_ENTER;
								GameScreenStack[GameStackReadPointer]->Message(msg);
								SendSetupDisplay();
							}
						break;
						case SDLK_b:
							Gamma += 0.15f;
							if(Gamma >= 2.5f)
								Gamma = 1.0f;
							SDL_SetGamma(Gamma, Gamma, Gamma);
						break;
 						case SDLK_m:
							MultiSample ^= true;
							FixRes = true;
						break;
 						case SDLK_h:
							if(FullScr)
							{
								HighRes ^= true;
								FixRes = true;
							}
						break;
						case SDLK_RETURN:
						if(event.key.keysym.mod&(KMOD_LALT|KMOD_RALT))
							{
								FullScr ^= true;
								FixRes = true;
							}
						break;
						case SDLK_LALT:
						case SDLK_RALT:
 							if(event.key.keysym.mod&(KMOD_LCTRL|KMOD_RCTRL) && !FullScr)
								GrabMouse = false;
						break;
						case SDLK_LCTRL:
						case SDLK_RCTRL:
 							if(event.key.keysym.mod&(KMOD_LALT|KMOD_RALT) && !FullScr)
								GrabMouse = false;
						break;
					}
					
				break;
			}

			/* check if we need to fix the video resolution */
			if(FixRes)
			{
				if(FullScr)
				{
					GrabMouse = true;
					if(!SetupGFX(Width, Height, FullScr, MultiSample, HighRes))
					{
						FullScr = false;
						if(!SetupGFX(Width, Height, FullScr, MultiSample, HighRes))
						{
							MultiSample = false;
							SetupGFX(Width, Height, FullScr, MultiSample, HighRes);
						}
					}
				}
				else
				{
					if(!SetupGFX(Width, Height, FullScr, MultiSample, HighRes))
					{
						MultiSample = false;
						SetupGFX(Width, Height, FullScr, MultiSample, HighRes);
					}
				}

				/* send setup display messages */
				msg.Type = EBGF_SETUPDISPLAY;
				AttachedGame->Message(msg);
				if(GameScreenStack[GameStackReadPointer] != AttachedGame) GameScreenStack[GameStackReadPointer]->Message(msg);

				/* fix grab mouse maybe */
				if(GrabMouse)
					SDL_WarpMouse(gScreen->w >> 1, gScreen->h >> 1);
			}

			if(OldGrabMouse != GrabMouse)
			{
				SDL_ShowCursor(GrabMouse ? SDL_DISABLE : SDL_ENABLE);
			}

			/* pass it on too! */
			msg.Type = EBGF_SDLMESSAGE;
			msg.Data.SDLMessage.Event = &event;
			msg.Data.SDLMessage.Age = ScreenAge;
			GameScreenStack[GameStackReadPointer]->Message(msg);
		}

		// check mouse stuff
		if(GrabMouse)
		{
			int NewX, NewY;
			MouseButtons = SDL_GetMouseState(&NewX, &NewY);
			SDL_WarpMouse(gScreen->w >> 1, gScreen->h >> 1);
			MickeyX += NewX - (gScreen->w >> 1);
			MickeyY += NewY - (gScreen->h >> 1);
		}

		// request redraw
		msg.Data.Draw.Age = ScreenAge;
		msg.Type = EBGF_DRAW;
		if(GameScreenStack[GameStackReadPointer] != AttachedGame) GameScreenStack[GameStackReadPointer]->Message(msg);
		AttachedGame->Message(msg);

		// flip buffers Ñ execution will pause here if we're ahead of the display frame rate
		SDL_GL_SwapBuffers();

		// work out how many logic updates that makes
		Uint32 ElapsedTime = SDL_GetTicks();
		ElapsedTime -= OldTime;
		OldTime += ElapsedTime;

		// progress by at most ElapsedTime ms
		int ThousandTicks = (ElapsedTime * BeatsPerSecond) + SpareMS;
		int Ticks = ThousandTicks / 1000;
		SpareMS = ThousandTicks % 1000;
		while(Ticks-- && !CommandPointer)
		{
			msg.Type = EBGF_UPDATE;
			msg.Data.Update.Age = ScreenAge; ScreenAge++;
			AttachedGame->Message(msg);
			if(GameScreenStack[GameStackReadPointer] != AttachedGame) GameScreenStack[GameStackReadPointer]->Message(msg);
		}

		// process stack commands
		if(CommandPointer)
		{
			ScreenAge = 0;
			msg.Type = EBGF_EXIT;
			GameScreenStack[GameStackReadPointer]->Message(msg);

			while(CommandPointer)
			{
				CommandPointer--;
				switch(PendingCommands[CommandPointer])
				{
					case EBGF_DESCEND:
						/* descend means "go one down the stack" */
						GameStackReadPointer++;
					break;
					case EBGF_ASCEND:
						/* ascend means "go one up the stack, forget everything after me" */
						GameStackReadPointer--;
						EBGF_SyncWritePointer();
					break;
					case EBGF_REMOVE:
					{
						/* remove means "cut me out, shuffle everything after me up a spot" */
						delete GameScreenStack[GameStackReadPointer];
						int c = GameStackReadPointer;
						while(c < GameStackWritePointer)
						{
							GameScreenStack[c] = GameScreenStack[c+1];
							c++;
						}					
					}
					break;
					case EBGF_RESTART:
						/* restart means "don't do anything to the stack, but send me a new exit/enter" */
					break;
				}
			}

			msg.Type = EBGF_ENTER;
			GameScreenStack[GameStackReadPointer]->Message(msg);
			SendSetupDisplay();
		}
	}

	GameStackReadPointer = 0;
	EBGF_SyncWritePointer();

	/* save whatever settings we can */
	if(SaveHandle = fopen(SaveName, "wb"))
	{
		/* yes, that's right Ñ someone might score 4 billion points */
		fputc(HighScore&0xff, SaveHandle);
		fputc((HighScore >> 8)&0xff, SaveHandle);
		fputc((HighScore >> 16)&0xff, SaveHandle);
		fputc((HighScore >> 24)&0xff, SaveHandle);

		/* get highres, multisample & fullscr settings */
		Uint8 Flags = (HighRes ? 1 : 0) | (MultiSample ? 2 : 0) | (FullScr ? 4 : 0);
		fputc(Flags, SaveHandle);

		/* get window size */
		fputc(Width&0xff, SaveHandle);
		fputc((Width >> 8)&0xff, SaveHandle);
		fputc((Width >> 16)&0xff, SaveHandle);
		fputc((Width >> 24)&0xff, SaveHandle);
		fputc(Height&0xff, SaveHandle);
		fputc((Height >> 8)&0xff, SaveHandle);
		fputc((Height >> 16)&0xff, SaveHandle);
		fputc((Height >> 24)&0xff, SaveHandle);

		/* get gamma */
		Uint32 FixGamma = (int)(Gamma * 65536.0f);
		fputc(FixGamma&0xff, SaveHandle);
		fputc((FixGamma >> 8)&0xff, SaveHandle);
		fputc((FixGamma >> 16)&0xff, SaveHandle);
		fputc((FixGamma >> 24)&0xff, SaveHandle);

		fclose(SaveHandle);
	}
	// Cleanup
	Mix_CloseAudio();
	SDL_Quit();

	return 0;
}
