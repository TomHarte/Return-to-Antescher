#include "Instructions.h"
#include "Gameplay.h"

#define TOTAL_TIME	6000

void CInstructions::Message(EBGF_Message &Message)
{
	switch(Message.Type)
	{
		default: break;
		case EBGF_SETUPDISPLAY:
			glPushAttrib(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
		break;
		case EBGF_EXIT:
			glPopAttrib();
		break;
		case EBGF_ENTER:
			TriggerSound(SOUND_PING);
		break;
		case EBGF_DRAW:
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glColor3f(1, 1, 1);
			
			int Stage = Message.Data.Draw.Age / 1500;
			int Offset = Message.Data.Draw.Age % 1500;
			float Angle = 0;
			
			if(Offset < 120) Angle = -120 + Offset;
			if(Offset > 1500 - 64) Angle = Offset - (1500 - 64);

			glPushMatrix();
			glTranslatef(-5, 4, 10);
			glRotatef(Angle, 0, 1, 0);

			switch(Stage)
			{
				case 0:
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Instructions");

					glTranslatef(0, -(EBGF_TextHeight() + 0.2f), 0);
					glScalef(0.6, 0.6, 1);

					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "After 23 years, the dreaded city of");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Antescher has been reopened.");

					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Now a land of perpetual night, only");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "one person has been brave enough");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "to return.");

					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Armed with a lantern and a mineral");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "sensor, netcat has headed into the");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "city to collect its valuable resources.");
				break;
				case 1:
					glScalef(0.6, 0.6, 1);

					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Use the mouse to look around. Use the");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "mouse button to throw the lantern.");

					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Click inside the game window to enable");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "mouse capture. Press ctrl and alt to");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "free the mouse.");

					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Use W, A, S and D to navigate, and");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "the space bar to jump.");

					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Use the light to find the green minerals,");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "but be careful! Legend says that ");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "the light will also attract the beast.");
				break;

				case 2:
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Other Controls");

					glTranslatef(0, -(EBGF_TextHeight() + 0.2f), 0);
					glScalef(0.6, 0.6, 1);

					glPushMatrix();
					glTranslatef(1.5, 0, 0);
					EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "Alt and");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "enter");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "H");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "B");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "ESC");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "M");
					glPopMatrix();
					
					glTranslatef(4.5, 0, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Toggle fullscreen mode");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Toggle high resolution graphics");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "in fullscreen mode");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Adjust screen brightness");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Return to the title screen, or");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "quit game from the title screen");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Toggle multisampling");

				break;
				case 3:
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Credits");

					glTranslatef(0, -(EBGF_TextHeight() + 0.2f), 0);
					glScalef(0.6, 0.6, 1);

					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Programming by Thomas Harte");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "for the Allegro.cc ChristmasHack 2006.");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Game map taken from Ant Attack, by");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Sandy White 1983.");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Sounds sourced from the freesound");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, " project, freesound.iua.upf.edu");
					glTranslatef(0, -EBGF_TextHeight()*1.5f, 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "ChristmasHack organised by");
					glTranslatef(0, -EBGF_TextHeight(), 0);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "Jakub Wasilewski");

				break;
			}
			
			glPopMatrix();
		}
		break;
		case EBGF_UPDATE:
		{
			if(Message.Data.Update.Age > TOTAL_TIME)
				EBGF_DoStackCommand(EBGF_ASCEND);
		}
		break;
		case EBGF_SDLMESSAGE:
			if(Message.Data.SDLMessage.Event->type == SDL_KEYDOWN && Message.Data.SDLMessage.Event->key.keysym.sym == SDLK_SPACE)
			{
				EBGF_DoStackCommand(EBGF_ASCEND);
			}
		break;
	}
}
