#include "Deathscreen.h"
#include "Gameplay.h"

#include "Instructions.h"

#define TOTAL_TIME	4500

void CDeathScreen::Message(EBGF_Message &Message)
{
	switch(Message.Type)
	{
		default: break;
		case EBGF_SETUPDISPLAY:
			glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_POLYGON_BIT);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
		break;
		case EBGF_EXIT:
			glPopAttrib();
		break;
		case EBGF_DRAW:
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glColor3f(1, 0, 0);

			glPushMatrix();
				glTranslatef(0, 0, 8.0f);
				glRotatef(-(float)Message.Data.Draw.Age*0.75f, 0, 0, 1);
				glRotatef((float)Message.Data.Draw.Age, 0.25f, 0.5f, 0);
				EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "Game Over");
			glPopMatrix();
		break;
		case EBGF_SDLMESSAGE:
			if(Message.Data.SDLMessage.Age >= 150 && (Message.Data.SDLMessage.Event->type == SDL_KEYUP || Message.Data.SDLMessage.Event->type == SDL_MOUSEBUTTONUP))
				EBGF_DoStackCommand(EBGF_ASCEND);
		break;
		case EBGF_ENTER:
			TriggerSound(SOUND_GAMEOVER);
		break;
	}
}
