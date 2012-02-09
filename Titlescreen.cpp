#include "Titlescreen.h"
#include "Gameplay.h"
#include "Instructions.h"

#define FIRST_MESSAGE_TIME	150
#define DROP_TIME			100
#define DROP_DEPTH			10.0f
#define TOTAL_TIME			1000

#define LINE1_TEXT			"Return to"
#define LINE2_TEXT			"Antescher"

void CTitlescreen::Message(EBGF_Message &Message)
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
		case EBGF_UPDATE:
		{
			if(Message.Data.Update.Age > TOTAL_TIME)
			{
				CInstructions *instr = new CInstructions;
				EBGF_PushScreen(instr);
				EBGF_DoStackCommand(EBGF_DESCEND);
			}
//				EBGF_DoStackCommand(EBGF_RESTART);
		}
		break;

		case EBGF_DRAW:
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if(Message.Data.Update.Age < FIRST_MESSAGE_TIME)
			{
				float Offset = (float)Message.Data.Update.Age / (float)FIRST_MESSAGE_TIME;
				float TextWidth = EBGF_TextWidth(LINE1_TEXT);

				glColor4f(1, 1, 1, 0.7f);

				glPushMatrix();
					glTranslatef(-2.0f + Offset*(TextWidth+4.0f), 0, 1);
					EBGF_PrintText(EBGFTA_RIGHT, EBGFTA_CENTRE, LINE1_TEXT);
				glPopMatrix();
				glPushMatrix();
					glTranslatef(2.0f-Offset*(TextWidth+4.0f), 0, 1);
					EBGF_PrintText(EBGFTA_LEFT, EBGFTA_CENTRE, LINE2_TEXT);
				glPopMatrix();
			}
			else
			{
				if(Message.Data.Draw.Age < FIRST_MESSAGE_TIME+DROP_TIME)
				{
					float Depth = (Message.Data.Update.Age - FIRST_MESSAGE_TIME) / (float)DROP_TIME;
					Depth *= DROP_DEPTH;

					glColor3f(1, 1, 1);
					glPushMatrix();
						glTranslatef(0, 0, Depth);
						EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, LINE1_TEXT);
						EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, LINE2_TEXT);
					glPopMatrix();
				}
				else
				{
					glColor3f(1, 1, 1);
					glPushMatrix();
						glTranslatef(0, 0, DROP_DEPTH);
						EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, LINE1_TEXT);
						EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, LINE2_TEXT);
						
						glPushMatrix();
							glTranslatef(0, EBGF_TextHeight()*4.0f, 0);
							glScalef(0.5, 0.5, 0.5);
							EBGF_PrintText(EBGFTA_RIGHT, EBGFTA_TOP, "High score %d  ", HighScore);
							EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "  Last score %d", LastScore);
						glPopMatrix();

							glTranslatef(0, -EBGF_TextHeight()*4.0f, 0);
							glScalef(0.5, 0.5, 1);

							float intensity = 1.0f;
							int Time = Message.Data.Update.Age - (FIRST_MESSAGE_TIME+DROP_TIME);

							switch(Time/50)
							{
								case 0:
								case 1:
								case 2:
								case 3:
									intensity = (float)Time / 50.0f;
									glColor4f(0.8, 0.5, 1, intensity);
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "Programmed by Thomas Harte 2006");

									glTranslatef(0, 2, 0);
									glScalef(0.8, 0.8, 1);
									glColor4f(0.5, 0.5, 0.5, intensity);
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "A game of light and dark");
								break;
								case 4:
									intensity = (float)(Time - 200) / 50.0f;
									glColor4f(0.8, 0.5, 1, 1.0f - intensity);
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "Programmed by Thomas Harte 2006");
									glColor4f(0.8, 0.5, 1, intensity);
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "Press Space to start");

									glTranslatef(0, 2, 0);
									glScalef(0.8, 0.8, 1);
									glColor4f(0.5, 0.5, 0.5, 1.0f - intensity);
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "A game of light and dark");
								break;
								default:
									glColor4f(0.8, 0.5, 1, 1);
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_CENTRE, "Press Space to start");
								break;
							}

					glPopMatrix();
				}
			}
		}
		break;

		case EBGF_SDLMESSAGE:
			if(Message.Data.SDLMessage.Event->type == SDL_KEYUP && Message.Data.SDLMessage.Event->key.keysym.sym == SDLK_SPACE)
			{
				CGameplay *Gamescr = new CGameplay;
				EBGF_PushScreen(Gamescr);
				EBGF_DoStackCommand(EBGF_DESCEND);
			}
		break;
	}
}
