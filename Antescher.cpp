#include "Antescher.h"
#include <stdio.h>
#include <math.h>

Uint8 Map[128][128];

bool LoadMap()
{
	FILE *mapfile;
	mapfile = fopen("map.dat", "rb");
	if(!mapfile) return false;
	
	/* not hard really... */
	for(int y = 127; y >= 0; y--)
	{
		for(int x =0; x < 128; x++)
			Map[x][y] = fgetc(mapfile);
	}

	fclose(mapfile);
	return true;
}

bool DoMovement(float *Pos, float *Vec, float *Size)
{
	/* HORRID QUICK FIX: check if Vec is too much to handle, if so recurse */
	if(
		(fabs(Vec[0]) > Size[SIDE]) ||
		(fabs(Vec[1]) > Size[SIDE]) ||
		(fabs(Vec[2]) > Size[TOP]) ||
		(fabs(Vec[3]) > Size[BOTTOM])
	)
	{
		float TempVec[3] = {Vec[0] * 0.5f, Vec[1]*0.5f, Vec[2]*0.5f};
		
		bool res = DoMovement(Pos, TempVec, Size);
		res |= DoMovement(Pos, TempVec, Size);
		Vec[0] = TempVec[0] * 2.0f;
		Vec[1] = TempVec[1] * 2.0f;
		Vec[2] = TempVec[2] * 2.0f;
		return res;
	}

	/* okay, we can cope, carry on... */
	bool OnGround = false;

	/* base assumption: movement was fine */
	Pos[0] += Vec[0];
	Pos[1] += Vec[1];
	Pos[2] += Vec[2];

	/* Get (StartX, StartY, StartZ) - the cube in which the centre of Pos lies */
	int StartX = (int)(Pos[0] + 0.5f), StartY = (int)(Pos[1]+ 0.5f), StartZ = (int)(Pos[2] + 0.5f);
	
	/* Get (OffX, OffY, OffZ) - the offset of Pos within its cube */
	float OffX = (float)fmod(Pos[0] + 0.5f, 1.0f), OffY = (float)fmod(Pos[1] + 0.5f, 1.0f), OffZ = (float)fmod(Pos[2] + 0.5f, 1.0f);

	/* Figure out (SearchLX, SearchLY) and (SearchHX, SearchHY) which are the top left and bottom right coordinates of
	the interesting part of the level map for this object */
	int SearchLX, SearchHX;
	int SearchLY, SearchHY;

	SearchLX = StartX;
	SearchHX = StartX;
	SearchLY = StartY;
	SearchHY = StartY;

	if(OffX < Size[SIDE]) SearchLX--;
	if(OffX > 1.0f - Size[SIDE]) SearchHX++;
	if(SearchLX < 0) SearchLX = 0;
	if(SearchHX > 127) SearchHX = 127;

	if(OffY < Size[SIDE]) SearchLY--;
	if(OffY > 1.0f - Size[SIDE]) SearchHY++;
	if(SearchLY < 0) SearchLY = 0;
	if(SearchHY > 127) SearchHY = 127;
	
//	printf("[%d %d]: %d %d %d %d\n", StartX, StartY, SearchLX, SearchLY, SearchHX, SearchHY);
	/* if completely outside map, job is quite easy... */
	if(
		(SearchHY < 0) || (SearchHX < 0) || (SearchLX > 127) || (SearchLY > 127)
	)
	{
		if(Pos[2] + 0.5f < Size[BOTTOM])
		{
			Pos[2] = Size[BOTTOM] - 0.5f;
			Vec[2] = 0;
			OnGround = true;
		}
	}
	else
	{
		/* Figure out Mask1 and Mask2 - both are used for &ing with the map. Mask1 is equivalent to the current height of
		Pos, Mask2 is either the row above or the row below depending which the object may be interacting with */
		int Mask1, Mask2;
		Mask2 = Mask1 = 1 << StartZ;
		if(OffZ < Size[BOTTOM] - 0.1f) Mask2 = Mask1 >> 1;
		if(OffZ > 1.0f - Size[TOP] + 0.1f) Mask2 = Mask1 << 1;

		/* check for collisions on the left side */
		if(SearchLX != StartX)
		{
			for(int y = SearchLY; y <= SearchHY; y++)
			{
				if(
					(
						(Map[SearchLX][y] & Mask1) && 
						!(Map[StartX][y] & Mask1)
					) ||
					(
						(Map[SearchLX][y] & Mask2) && 
						!(Map[StartX][y] & Mask2)
					)
				)
				{
					Pos[0] += Size[SIDE] - OffX;
					Vec[0] = -Vec[0] * 0.1f;
					SearchLX = StartX;
					break;
				}
			}
		}

		/* check for collisions on the right side */
		if(SearchHX != StartX)
		{
			for(int y = SearchLY; y <= SearchHY; y++)
			{
				if(
					(
						(Map[SearchHX][y] & Mask1) && 
						!(Map[StartX][y] & Mask1)
					) ||
					(
						(Map[SearchHX][y] & Mask2) && 
						!(Map[StartX][y] & Mask2)
					)
				)
				{
					Pos[0] -= OffX - (1.0f - Size[SIDE]);
					Vec[0] = -Vec[0] * 0.1f;
					SearchHX = StartX;
					break;
				}
			}
		}

		/* check for collisions on the top side */
		if(SearchLY != StartY)
		{
			for(int x = SearchLX; x <= SearchHX; x++)
			{
				if(
					(
						(Map[x][SearchLY] & Mask1) && 
						!(Map[x][StartY] & Mask1)
					) ||
					(
						(Map[x][SearchLY] & Mask2) && 
						!(Map[x][StartY] & Mask2)
					)
				)
				{
					Pos[1] += Size[SIDE] + 0.005f - OffY;
					Vec[1] = -Vec[1] * 0.1f;
					SearchLY = StartY;
					break;
				}
			}
		}

		/* check for collisions on the bottom side */
		if(SearchHY != StartY)
		{
			for(int x = SearchLX; x <= SearchHX; x++)
			{
				if(
					(
						(Map[x][SearchHY] & Mask1) && 
						!(Map[x][StartY] & Mask1)
					) ||
					(
						(Map[x][SearchHY] & Mask2) && 
						!(Map[x][StartY] & Mask2)
					)
				)
				{
					Pos[1] -= OffY - (1.0f - Size[SIDE]);
					Vec[1] = -Vec[1] * 0.1f;
					SearchHY = StartY;
					break;
				}
			}
		}

		/* determine a more stringet Mask2 - this is just about avoiding logical circular arguments */
		if(OffZ < Size[BOTTOM]) Mask2 = Mask1 >> 1;
		if(OffZ > 1.0f - Size[TOP]) Mask2 = Mask1 << 1;

		/* has the object landed on anything? */
		if(OffZ < Size[BOTTOM] && Vec[2] < 0)
		{
			bool LayerBelow = false;

			if(!StartZ) LayerBelow = true;
			else
			{
				for(int x = SearchLX; x <= SearchHX; x++)
					for(int y = SearchLY; y <= SearchHY; y++)
					{
						if(Map[x][y] & Mask2)
							LayerBelow = true;
					}
			}

			if(LayerBelow)
			{
				Pos[2] += Size[BOTTOM] - OffZ;
				Vec[2] = 0;
				OnGround = true;
			}
		}

		/* clip at top */
		if(OffZ > 1.0f - Size[TOP])
		{
			for(int x = SearchLX; x <= SearchHX; x++)
				for(int y = SearchLY; y <= SearchHY; y++)
				{
					if(Map[x][y] & Mask2)
					{
						Pos[2] -= OffZ - (1.0f - Size[TOP]);
						Vec[2] = 0;
						break;
					}
				}
		}
	}


	return OnGround;
}
