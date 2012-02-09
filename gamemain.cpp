#include "ebgf.h"
#include "antescher.h"
#include "Titlescreen.h"
#include "ConvexObject.h"
#include "Gameplay.h"
#include "SDL_mixer.h"

class CMyGame: public CGame
{
	public:
		void Message(EBGF_Message &Message);
		CGameScreen *GetFirstScreen() {return new CTitlescreen;}
};

CMyGame MyGame;
int LastScore = 0, HighScore = 0;

Mix_Chunk *Samples[7];
void TriggerSound(int id)
{
	Mix_PlayChannel(id, Samples[id], 0);
}

void SetDroneVolume(float f)
{
	if(f < 0) f = 0;
	Mix_Volume(2, (int)((float)MIX_MAX_VOLUME * f));
}

void CMyGame::Message(EBGF_Message &Message)
{
	switch(Message.Type)
	{
		default: break;
		case EBGF_PROGSTART:
			/* load map */
			LoadMap();

			/* set up basic convex objects */
			CubeObject.SetCube();
			SphereObject.SetSphere();
			
			/* load settings */
			LastScore = 0;
			
			/* load sounds */
			Samples[0] = Mix_LoadWAV("Ping.wav");
			Samples[1] = Mix_LoadWAV("Collect.wav");
			Samples[2] = Mix_LoadWAV("drone.wav");
			Samples[3] = Mix_LoadWAV("throw.wav");
			Samples[4] = Mix_LoadWAV("jump.wav");
			Samples[5] = Mix_LoadWAV("gameover.wav");
			Samples[6] = Mix_LoadWAV("warning.wav");
			Mix_AllocateChannels(7);
			Mix_Volume(3, 16);
			Mix_Volume(4, 16);
			SetDroneVolume(0);
			Mix_PlayChannel(2, Samples[2], -1);
		break;
		case EBGF_PROGEND:
		{
			/* save settings */
			int c = 7;
			while(c--)
				Mix_FreeChunk(Samples[c]);
		}
		break;
		case EBGF_SETUPDISPLAY:
			SDL_WM_SetCaption("Return to Antescher", "Return to Antescher");
		break;
	}
}

CGame *GetGame()
{
	return &MyGame;
}