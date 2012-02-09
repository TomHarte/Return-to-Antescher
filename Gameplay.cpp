#include "Gameplay.h"
#include "antescher.h"
#include <math.h>
#include "cubedrawing.h"
#include "ConvexObject.h"
#include "Deathscreen.h"

#define MINERAL_MOTION_RADIUS	0.3f
Uint8 CountTable[64];

void DrawRow(int x1, int y, int x2)
{
	glPushMatrix();
	for(int x = x1; x <= x2; x++)
	{
		Uint8 Data;
		
		if(Data = Map[x][y])
		{
			Uint8 LeftData = (x > 0) ? Map[x-1][y] : 0;
			Uint8 RightData = (x < 127) ? Map[x+1][y] : 0;
			Uint8 TopData = (y > 0) ? Map[x][y-1] : 0;
			Uint8 BottomData = (y < 127) ? Map[x][y+1] : 0;

			Data = (Data << 1) | 1;
			
			glPushMatrix();
			while(Data&~1)
			{
				if(Data&2)
					DrawCube(LeftData, RightData, TopData, BottomData, Data);

				Data >>= 1;
				LeftData >>= 1;
				RightData >>= 1;
				TopData >>= 1;
				BottomData >>= 1;
				glTranslatef(0, 0, -1);
			}
			glPopMatrix();
		}
		glTranslatef(1, 0, 0);
	}
	glPopMatrix();
}

#define DRAW_DISTANCE 200

void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	/* draw rows within triangle */

	/* quick hack: use bounding box */
	float lowx = x1;
	if(x2 < lowx) lowx = x2;
	if(x3 < lowx) lowx = x3;

	float highx = x1;
	if(x2 > highx) highx = x2;
	if(x3 > highx) highx = x3;

	float lowy = y1;
	if(y2 < lowy) lowy = y2;
	if(y3 < lowy) lowy = y3;

	float highy = y1;
	if(y2 > highy) highy = y2;
	if(y3 > highy) highy = y3;
	
	int sx = (int)(lowx - 0.5f);
	int ex = (int)(highx + 0.5f);
	int sy = (int)(lowy - 0.5f);
	int ey = (int)(highy + 0.5f);
	
	if(ey < 0) return;
	if(ex < 0) return;
	if(sx > 128) return;
	if(sy > 128) return;
	
	sx --;
	sy --;
	ex ++;
	ey++;
	
	if(sx < 0) sx = 0;
	if(sy < 0) sy = 0;
	if(ex > 127) ex = 127;
	if(ey > 127) ey = 127;

	glTranslatef(sx, sy, 0);
	for(int y = sy; y <= ey; y++)
	{
		DrawRow(sx, y, ex);
		glTranslatef(0, 1, 0);
	}
}

void DrawSweep(float x, float y, float startang, float endang)
{
	/* if angle is too big, recurse */
	if(endang - startang > 90)
	{
		DrawSweep(x, y, startang, (startang+endang)*0.5f);
		DrawSweep(x, y, (startang+endang)*0.5f, endang);
	}
	else
	{
		float startangrad = startang * 3.141592654f / 180.0f;
		float endangrad = endang * 3.141592654f / 180.0f;
		DrawTriangle(x, y, x + DRAW_DISTANCE*sin(startangrad), y + DRAW_DISTANCE*cos(startangrad), x + DRAW_DISTANCE*sin(endangrad), y + DRAW_DISTANCE*cos(endangrad));
	}
}


#define MOVE_DISTANCE	0.125f
#define THROW_SPEED		0.25f
#define SIDE_CLIP	0.2f
#define TOP_CLIP	0.2f
#define BOTTOM_CLIP	0.5f

#define NeighbourCheck(xp, yp)\
	{\
		Uint8 NeighbourMask = 0;\
		switch(z)\
		{\
			case 0:\
				if(!(Map[xp][yp]&1)) NeighbourMask |= 2;\
			break;\
			case 1:\
				if(!(Map[xp][yp]&Mask) && Map[xp][yp]&(Mask >> 1)) NeighbourMask |= 2;\
				if(!(Map[xp][yp]&(Mask >> 1)) && !(Map[xp][yp]&Mask)) NeighbourMask |= 1;\
			break;\
			default:\
				if(!(Map[xp][yp]&Mask) && Map[xp][yp]&(Mask >> 1)) NeighbourMask |= 2;\
				if(!(Map[xp][yp]&(Mask >> 1)) && !(Map[xp][yp]&Mask) && (Map[xp][yp]&(Mask >> 2))) NeighbourMask |= 1;\
			break;\
		}\
		if((Map[xp][yp]&Mask) && !(Map[xp][yp]&(Mask << 1)) && !(Map[x][y]&(Mask << 1))) NeighbourMask |= 4;\
		\
		if((NeighbourMask&1) && PlayerCanReachR(xp, yp, z-1)) { return true;}\
		if((NeighbourMask&2) && PlayerCanReachR(xp, yp, z)) { return true;}\
		if((NeighbourMask&4) && PlayerCanReachR(xp, yp, z+1)) { return true;}\
	}

bool CGameplay::PlayerCanReach(int x, int y, int z)
{
	/* clear check map */
	for (int iy = 0; iy < 128; iy++)
		memcpy(CheckMap[iy], Map[iy], 128);
		
	return PlayerCanReachR(x, y, z);
}

bool CGameplay::PlayerCanReachR(int x, int y, int z)
{
	Uint8 Mask = 1 << z;
	
	/* if this square has been checked, then: no */
	if(CheckMap[x][y] & Mask)
		return false;
	CheckMap[x][y] |= Mask;

	/* if this square is on the ground bordering the edge of the map then we've done it! */
	if(
		(!x || !y || (x == 127) || (y == 127)) && (z < 2)
	)
	{
		return true;
	}

	{
		if(x > 0)
		{
			NeighbourCheck(x-1, y);
		}
		if(x < 127)
		{
			NeighbourCheck(x+1, y);
		}
		if(y > 0)
		{
			NeighbourCheck(x, y-1);
		}
		if(y < 127)
		{
			NeighbourCheck(x, y+1);
		}
	}

	/* can't, sorry */
	return false;
}

void CGameplay::RePlaceMineral()
{
	while(1)
	{
		/* pick a random map square */
		int MapX, MapY;
		MapX = ((Uint64)rand() * 128) / RAND_MAX;
		MapY = ((Uint64)rand() * 128) / RAND_MAX;
		
		Uint8 Data = Map[MapX][MapY];
		
		if(!Data)
		{
			/* if empty column, small chance of accept */
			if(!(rand() / (RAND_MAX >> 4)))
			{
				MineralPos[0] = MapX;
				MineralPos[1] = MapY;
				MineralPos[2] = 0;
//				break;
			}
		}
		else
		{
			/* find number of top platforms in square */
			int Platforms = 0;
			while(Data)
			{
				if(!Data&1) Platforms++;
				Data >>= 1;
			}
			Platforms++;

			/* pick a platform at random, turn that into a level */
			int ChosenPlatform = ((Uint64)rand() * Platforms) / RAND_MAX;
			int Height = 0;
			Data = Map[MapX][MapY];
			while(Data)
			{
				if(!(Data&1))
				{
					if(!ChosenPlatform) break;
					ChosenPlatform--;
				}
				Height++;
				Data >>= 1;
			}

			/* check if reachable... */
			if(PlayerCanReach(MapX, MapY, Height))
			{
				MineralPos[0] = MapX;
				MineralPos[1] = MapY;
				MineralPos[2] = Height;
				break;
			}
		}
	}
}

float CGameplay::DistanceToMineral()
{
	float DistanceVec[3] = {MineralPos[0] - PlayerPos[0], MineralPos[1] - PlayerPos[1], MineralPos[2] - PlayerPos[2]};
	return sqrt(DistanceVec[0]*DistanceVec[0] + DistanceVec[1]*DistanceVec[1] + DistanceVec[2]*DistanceVec[2]);
}

int ShadowVolumes;
float CubeGrid[100][100][6][3];

void CGameplay::CastLevel(float *TLightPos)
{
	int LowX = (int)(LightPos[0] - 0.5f) - LIGHT_WIDTH, LowY = (int)(LightPos[1] - 0.5f) - LIGHT_WIDTH;
	int HighX = LowX + LIGHT_WIDTH + LIGHT_WIDTH, HighY = LowY + LIGHT_WIDTH + LIGHT_WIDTH;

	if(LowX < 0) LowX = 0;
	if(LowY < 0) LowY = 0;
	if(HighX > 127) HighX = 127;
	if(HighY > 127) HighY = 127;

	ShadowVolumes = 0;

	/* clear cube grid */
	int x, y, z;
	for(x = LowX; x <= HighX; x++)
		for(y = LowY; y <= HighY; y++)
			for(z = 0; z < 6; z++)
				CubeGrid[x - LowX][y - LowY][z][0] = -1;

	/* work out cubes with simple "grow to fit" algorithm */
	for(y = LowY; y <= HighY; y++)
		for(x = LowX; x <= HighX; x++)
		{
			if(Map[x][y])
			{
				Uint8 Shifter = Map[x][y];
				float Width = 1;

				/* possible cheat: how many x's can we actually do at once here? */
				while(x < HighX && Map[x+1][y] == Map[x][y])
				{
					Width++;
					x++;
				}
/*				if(x < HighX && Map[x+1][y] == Map[x][y])
				{
					Width++;
					x++;
				}*/
				z = 0;

				while(Shifter)
				{
					if(Shifter&1)
					{
//						ShadowVolumes++;
						CubeGrid[x - LowX][y - LowY][z][0] = Width;
						CubeGrid[x - LowX][y - LowY][z][1] = 1;
						switch(Shifter)
						{
							default:
								CubeGrid[x - LowX][y - LowY][z][2] = 1;
								z++;
								Shifter >>= 1;
							break;
							case 3:
								CubeGrid[x - LowX][y - LowY][z][2] = 2;
								z+=2;
								Shifter >>= 2;
							break;
							case 7:
								CubeGrid[x - LowX][y - LowY][z][2] = 3;
								z+=3;
								Shifter >>= 3;
							break;
							case 15:
								CubeGrid[x - LowX][y - LowY][z][2] = 4;
								z+=4;
								Shifter >>= 4;
							break;
							case 31:
								CubeGrid[x - LowX][y - LowY][z][2] = 5;
								z+=5;
								Shifter >>= 5;
							break;
							case 63:
								CubeGrid[x - LowX][y - LowY][z][2] = 6;
								z+=6;
								Shifter >>= 6;
							break;
						}
					}
					else
					{
						z++;
						Shifter >>= 1;
					}
				}
			}
		}
		
	/* compact in y, if possible */
	for(x = LowX; x <= HighX; x++)
		for(z = 0; z < 6; z++)
		{
			y = LowY;

			while(y < HighY)
			{
				/* find next cuboid that might be compactible*/
				while(y < HighY && CubeGrid[x - LowX][y - LowY][z][0] < 0)
					y++;

				/* compact */
				int StartY = y;
				while(y <= HighY && 
						(
							CubeGrid[x - LowX][y+1 - LowY][z][0] == CubeGrid[x - LowX][StartY - LowY][z][0] &&
							CubeGrid[x - LowX][y+1 - LowY][z][2] == CubeGrid[x - LowX][StartY - LowY][z][2]
						)
					)
				{
					CubeGrid[x - LowX][StartY - LowY][z][1]++;
					y++;
					CubeGrid[x - LowX][y - LowY][z][0] = -1;
				}

				/* start at next position for next time */
				y++;
			}
		}

	/* and draw them... */
	for(y = LowY; y <= HighY; y++)
		for(x = LowX; x <= HighX; x++)
			for(z = 0; z < 6; z++)
				if(CubeGrid[x - LowX][y - LowY][z][0] > 0)
				{
					float CubePos[3] = {
							x - ((CubeGrid[x - LowX][y - LowY][z][0]-1) * 0.5f),
							y + ((CubeGrid[x - LowX][y - LowY][z][1]-1) * 0.5f),
							-(z + ((CubeGrid[x - LowX][y - LowY][z][2]-1) * 0.5f))
					};
					DrawCubeShadowVolume(TLightPos, CubePos,
							CubeGrid[x - LowX][y - LowY][z][0],
							CubeGrid[x - LowX][y - LowY][z][1],
							CubeGrid[x - LowX][y - LowY][z][2]);
					ShadowVolumes++;
				}
}

void CGameplay::Message(EBGF_Message &Message)
{
	switch(Message.Type)
	{
		case EBGF_EXIT:
			delete Monster;
			glPopAttrib();
			glDisable(GL_POLYGON_OFFSET_FILL);
			SetDroneVolume(0);
		break;
		case EBGF_SETUPDISPLAY:
		{
			glPushAttrib(GL_LIGHTING_BIT|GL_STENCIL_BUFFER_BIT);

			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glDisable(GL_STENCIL_TEST);
			glEnable(GL_POLYGON_OFFSET_FILL);

			// set attenuation
			glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
			glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.15f);
			glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.06f);
		}
		break;
		case EBGF_DRAW:
		{
			/* update ambient light */
			float LightIntensity = 0.2f + cos((float)Message.Data.Update.Age * 2 * 3.141592654f / (3 * 60 * 100.0f)*0.2f);
			GLfloat AmbientLight[] = { LightIntensity, LightIntensity, LightIntensity, 1.0f };
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, AmbientLight);
			
			/* work out current position of mineral 'animation' */
			float AniMineralPos[3], MineralAngle;
			MineralAngle = (float)Message.Data.Update.Age * 2 * 3.141592654f / 100.0f;
			AniMineralPos[0] = MineralPos[0] + sin(MineralAngle) * MINERAL_MOTION_RADIUS;
			AniMineralPos[1] = MineralPos[1] + cos(MineralAngle) * MINERAL_MOTION_RADIUS;
			AniMineralPos[2] = -MineralPos[2] + sin(MineralAngle * 0.5f) * MINERAL_MOTION_RADIUS;
			
			/* clear all buffers */
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			/* ... and draw! */
			glPushMatrix();
			glPushAttrib(GL_LIGHTING_BIT|GL_STENCIL_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

				/* set camera in correct place */
				glRotatef(PlayerAngle[1], 1, 0, 0);
				glRotatef(PlayerAngle[0], 0, 0, 1);
				glTranslatef(-PlayerPos[0], -PlayerPos[1], PlayerPos[2]);
				float HFOV = EBGF_GetHFOV();

				/* render once with light on, to get correct z-depths and lit scene */

					/* set light position */
					float TLightPos[4] = {LightPos[0], LightPos[1], -LightPos[2], 1};

					/* being chased? */
					if(MSGPointer == 4)// && ((Message.Data.Draw.Age >> 2)&3))
					{
						float *Temp = Monster->GetPos();
						TLightPos[0] = Temp[0];
						TLightPos[1] = Temp[1];
						TLightPos[2] = -Temp[2];
						// red light, slightly brighter
						GLfloat ambientLight[] = { 0.3f, 0.0f, 0.0f, 1.0f };
						GLfloat diffuseLight[] = { 0.9f, 0.0f, 0.0, 1.0f };
						GLfloat specularLight[] = { 0.6f, 0.0f, 0.0f, 1.0f };

						glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
						glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
						glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
					}
					else
					{
						// normal white light
						GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
						GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
						GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };

						glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
						glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
						glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
					}

					glLightfv(GL_LIGHT0, GL_POSITION, TLightPos);

					/* load normal "grey" material */
					float GreyDiffuse[4] = {0.8, 0.8, 0.8, 1}, GreyAmbient[4] = {0.1, 0.1, 0.1, 1};
					float GreenDiffuse[4] = {0, 1, 0, 1}, GreenAmbient[4] = {0, 0.1, 0, 1};
					glPolygonOffset(0, 0);

					/* add mineral */
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GreenDiffuse);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, GreenAmbient);
					glPushMatrix();
						glTranslatef(AniMineralPos[0], AniMineralPos[1], AniMineralPos[2]);
						glScalef(0.4f, 0.4f, 0.4f);
						SphereObject.Draw();
					glPopMatrix();

					/* add monster!!! */
					Monster->Draw();

					/* draw floor */
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GreyDiffuse);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, GreyAmbient);
					glNormal3f(0, 0, 1);
					glColor3f(0.5, 0.5, 0.5);
					glBegin(GL_QUADS);
					for(int x = -8; x < 40; x++)
					{
						for(int y = -8; y < 40; y++)
						{
							glVertex3f(x * 4, y * 4, 0.5);
							glVertex3f((x+1) * 4, y * 4, 0.5);
							glVertex3f((x+1) * 4, (y+1) * 4, 0.5);
							glVertex3f(x * 4, (y+1) * 4, 0.5);
						}
					}
					glEnd();
					
					glColor3f(1, 1, 1);
					/* draw part of map that may be affected by lighting */
					glPolygonOffset(0, 0);
					glPushMatrix();
						DrawSweep(PlayerPos[0], PlayerPos[1], PlayerAngle[0] - HFOV*0.5f, PlayerAngle[0] + HFOV*0.5f);
					glPopMatrix();
					glPushMatrix();
						glTranslatef(LightPos[0], LightPos[1], -LightPos[2]);
						glScalef(0.4f, 0.4f, 0.4f);
						glColor3f(0.8, 0.8, 0.8);		
						DrawInvertedCube();
					glPopMatrix();

				/* render shadow volumes, to get correct stencil - no need for any lighting */
					glEnable(GL_STENCIL_TEST);
					glStencilFunc(GL_EQUAL, 0, 0);	//don't listen to stencil for drawing


					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
					
					glDisable(GL_LIGHTING);
					glPolygonOffset(1, 1);

						/* add shadow for potentially shadowy piece of map
						
							TODO: heavily optimise! */
							CastLevel(TLightPos);
									
						/* add shadow for mineral */
						DrawSphereShadowVolume(TLightPos, AniMineralPos, 0.2, 0.2, 0.2);
						
						/* add shadow for monster */
						Monster->ProjectShadow(TLightPos);


				/* re-render in stencil only, with light disabled - clear depth buffer to make sure everything draws */
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glEnable(GL_LIGHTING);
					glDisable(GL_LIGHT0);
					glDepthFunc(GL_ALWAYS);

					glStencilFunc(GL_NOTEQUAL, 0, 0xff);	// render if stencil position&1 == 1

					// black for level
					glPushMatrix();
						glLoadIdentity();	// back to identity

						/* VFOV = 45, so tan (45/2) = diff / 300 => diff = 300 * tan (45/2) = 124 */
						glColor3f(1, 1, 1);
						float Depth = 395.0f;
						float Height = Depth * tan( EBGF_GetVFOV() / 360.0f * 3.141592654f);
						float Width = Depth * tan( HFOV / 360.0f * 3.141592654f);

						glBegin(GL_QUADS);	// draw a single giant quad, so all places marked as shadow now become whatever the "unlit" grey is
							glVertex3f(-Width, -Height, Depth);
							glVertex3f(Width, -Height, Depth);
							glVertex3f(Width, Height, Depth);
							glVertex3f(-Width, Height, Depth);
						glEnd ();
					glPopMatrix();

					/* readd mineral */
					glDepthFunc(GL_LESS);
					glPolygonOffset(-1, -1);
					/* add mineral */
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GreenDiffuse);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, GreenAmbient);
					glPushMatrix();
						glTranslatef(AniMineralPos[0], AniMineralPos[1], AniMineralPos[2]);
						glScalef(0.4f, 0.4f, 0.4f);
						SphereObject.Draw();
					glPopMatrix();

					/* readd monster!!! */
					Monster->Draw();

					/* HUD */
					glDepthFunc(GL_ALWAYS);
					glDisable(GL_STENCIL_TEST);
					glDisable(GL_LIGHTING);
					glLoadIdentity();
					glPushMatrix();
						glTranslatef(0, 4, 10);
						glScalef(0.3, 0.3, 0.3);
					
						glColor3f(0, 0, 0.8);
						EBGF_PrintText(EBGFTA_LEFT, EBGFTA_TOP, "  Distance %0.2f", DistanceToMineral() / 2.0f);
						EBGF_PrintText(EBGFTA_RIGHT, EBGFTA_TOP, "%d Collected  ", CollectedMinerals);
//						glTranslatef(0, -EBGF_TextHeight(), 0);
//						EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_TOP, "%d shadow volumes", ShadowVolumes);
					glPopMatrix();
					
					if(Message.Data.Draw.Age < MSGStartTime+500 || (MSGPointer > 2))
					{
						float Intensity = 1.0f;
						if(Message.Data.Draw.Age > MSGStartTime + 450 && (MSGPointer < 3))
							Intensity = 1.0f - (float)(Message.Data.Draw.Age - (MSGStartTime + 450)) / 50.0f;

						glPushMatrix();
							glColor4f(0.8, 0, 0, Intensity);
							glTranslatef(0, -4, 10);
							glScalef(0.4, 0.4, 0.4);

							MonsterVeryClose = false;
							switch(MSGPointer)
							{
								case 0:
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, "The monster is roaming");
								break;
								case 1:
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, "The monster has spotted the light");
								break;
								case 2:
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, "The monster is catching his breath");
								break;
								case 3:
									EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, "The monster can smell you");
								break;
								case 4:
								{
									float *MonsterPos = Monster->GetPos();
									float Diff[3] = {PlayerPos[0] - MonsterPos[0], PlayerPos[1] - MonsterPos[1], PlayerPos[2] - MonsterPos[2]};
									float Distance = Diff[0]*Diff[0] + Diff[1]*Diff[1] + Diff[2]*Diff[2];
									
									if(Distance < 10*10)
									{
										EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, "RUN RUN RUN");
										MonsterVeryClose = true;
									}
									else
										EBGF_PrintText(EBGFTA_CENTRE, EBGFTA_BOTTOM, "The monster is coming for you!");
								}
								break;
							}
						glPopMatrix();
					}
			glPopAttrib();
			glPopMatrix();
		}
		break;
		case EBGF_ENTER:
		{
			EBGF_ClearMouseMickeys();
			PlayerPos[0] = 52.5f;
			PlayerPos[1] = -10.0f;
			PlayerPos[2] = 0;
			PlayerVelocity[0] = 0;
			PlayerVelocity[1] = 0;
			PlayerVelocity[2] = 0;
			PlayerAngle[0] = 0;
			PlayerAngle[1] = 90;
			PlayerSize[SIDE] = SIDE_CLIP;
			PlayerSize[TOP] = TOP_CLIP;
			PlayerSize[BOTTOM] = BOTTOM_CLIP;

			LightPos[0] = 53.5f;
			LightPos[1] = 0.0f;
			LightPos[2] = 8.0f;
			LightPos[3] = 1.0f;
			LightVelocity[0] = 0;
			LightVelocity[1] = 0;
			LightVelocity[2] = 0;
			LightSize[0] =
			LightSize[1] =
			LightSize[2] = 0.2f;

			MineralPos[0] = 51.5;
			MineralPos[1] = 0.0f;
			MineralPos[2] = 0;
			CollectedMinerals = 0;
			
			int c = 64;
			while(c--)
			{
				int count = 0;
				int sc = c;
				while(sc)
				{
					if(sc&1) count++;
					sc >>= 1;
				}
				CountTable[c] = count;
			}

			Monster = new CMonster;
			MonsterVeryClose = false;
		}
		break;
		case EBGF_UPDATE:
		{
			/* do all player input */
			Uint8 *KeyArray = SDL_GetKeyState(NULL);
			float CamAng = PlayerAngle[0] * 3.141592654f / 180.0f;

			/* move player */
			PlayerVelocity[2] -= 0.005f;

			PlayerVelocity[0] = PlayerVelocity[1] = 0;
			if(KeyArray[SDLK_DOWN] || KeyArray[SDLK_s])		{PlayerVelocity[0] += -MOVE_DISTANCE*sin(CamAng); PlayerVelocity[1] += -MOVE_DISTANCE*cos(CamAng);}
			if(KeyArray[SDLK_UP] || KeyArray[SDLK_w])		{PlayerVelocity[0] += MOVE_DISTANCE*sin(CamAng); PlayerVelocity[1] += MOVE_DISTANCE*cos(CamAng);}
			if(KeyArray[SDLK_RIGHT] || KeyArray[SDLK_d])	{PlayerVelocity[0] += MOVE_DISTANCE*cos(CamAng); PlayerVelocity[1] += -MOVE_DISTANCE*sin(CamAng);}
			if(KeyArray[SDLK_LEFT] || KeyArray[SDLK_a])		{PlayerVelocity[0] += -MOVE_DISTANCE*cos(CamAng); PlayerVelocity[1] += MOVE_DISTANCE*sin(CamAng);}

			int MouseX, MouseY;
			int MouseButtons;
			MouseButtons = EBGF_GetMouseMickeys(MouseX, MouseY);
			PlayerAngle[0] += (float)MouseX / 10.0f;
			PlayerAngle[1] -= (float)MouseY / 10.0f;
	
			if(PlayerAngle[1] < 0) PlayerAngle[1] = 0;
			if(PlayerAngle[1] > 180) PlayerAngle[1] = 180;

			if(DoMovement(PlayerPos, PlayerVelocity, PlayerSize))
			{
				if(KeyArray[SDLK_SPACE]) {PlayerVelocity[2] = 0.13f; TriggerSound(SOUND_JUMP);}
			}

			/* move light */
			LightVelocity[2] -= 0.002f;
			if(DoMovement(LightPos, LightVelocity, LightSize))
			{
				LightVelocity[0] = LightVelocity[1] = 0;

				if(MouseButtons)
				{
					TriggerSound(SOUND_THROW);
					float ElevAng = PlayerAngle[1] * 3.141592654f / 180.0f;
				
					LightVelocity[0] = sin(CamAng)*THROW_SPEED*sin(ElevAng);
					LightVelocity[1] = cos(CamAng)*THROW_SPEED*sin(ElevAng);
					LightVelocity[2] = -cos(ElevAng)*THROW_SPEED;
					LightPos[0] = PlayerPos[0];
					LightPos[1] = PlayerPos[1];
					LightPos[2] = PlayerPos[2];
				}
			}

			/* check if a mineral has been collected */
			if(DistanceToMineral() < 0.8f)
			{
				TriggerSound(SOUND_COLLECT);
				CollectedMinerals++;
				RePlaceMineral();
			}

			/* update monster */
			Monster->Update(PlayerPos, LightPos);
			if(Monster->StateChanged())
			{
				TriggerSound((Monster->State() == MS_CHASING) ? SOUND_WARNING : SOUND_PING);
				MSGStartTime = Message.Data.Update.Age;
				switch(Monster->State())
				{
					case MS_ROAMING:
						MSGPointer = 0;
					break;
					case MS_SEENLIGHT:
						MSGPointer = 1;
					break;
					case MS_RESTING:
						MSGPointer = 2;
					break;
					case MS_CANSMELL:
						MSGPointer = 3;
					break;
					case MS_CHASING:
						MSGPointer = 4;
					break;
				}
			}
			
			/* is player dead? */
			float *MonsterPos = Monster->GetPos();
			float Diff[3] = {PlayerPos[0] - MonsterPos[0], PlayerPos[1] - MonsterPos[1], PlayerPos[2] - MonsterPos[2]};
			float Distance = Diff[0]*Diff[0] + Diff[1]*Diff[1] + Diff[2]*Diff[2];
			if(Distance < 1)
			{
				LastScore = CollectedMinerals;
				if(LastScore > HighScore) HighScore = LastScore;
				
				CDeathScreen *DS = new CDeathScreen;
				EBGF_PushScreen(DS);
				EBGF_DoStackCommand(EBGF_REMOVE);
			}
			SetDroneVolume(1.0f - (Distance / (16.0f * 16.0f)));
		}
		break;
	}
}
