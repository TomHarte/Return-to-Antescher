#include "Monster.h"
#include "ConvexObject.h"
#include "cubedrawing.h"
#include "Antescher.h"
#include <math.h>

void CMonster::GetNewTarget(float *LightPos)
{
	/* 1 in 3 chance it'll go "close" to the light */
	if(!(rand() / (RAND_MAX /3)))
	{
		Target[0] = LightPos[0];
		Target[1] = LightPos[1];
		Target[2] = LightPos[2];

		/* something not much like the Gaussian distribution... */
		int c = 8;
		while(c--)
		{
			Target[0] += (flrand() - 0.5f) *32.0f / (float)(c+1);
			Target[1] += (flrand() - 0.5f) *32.0f / (float)(c+1);
			Target[2] += (flrand() - 0.5f) *6.0f / (float)(c+1);
		}
		Target[2] += 4.0f;	// so that the monster generally floats about when interested in the light
	}
	else
	{
		Target[0] = flrand() * 128.0f;
		Target[1] = flrand() * 128.0f;
		Target[2] = flrand() * 8.0f;
	}
}

CMonster::CMonster()
{
	StateChangedFlag = true;
	CurrentState = MS_ROAMING;
	StateTimer = 0;
	Pos[0] = 60;
	Pos[1] = 60;
	Pos[2] = 0;
	
	Vec[0] = Vec[1] = Vec[2] = 0;
	Size[0] = Size[1] = Size[2] = 0.5f;
	OnGround = true;
	Shape.SetSphere();
}

#define SMELL_DISTANCELOW		25
#define SMELL_DISTANCEHIGH		55
#define GIVEUP_DISTANCE			45
#define SMELL_TIME				700
#define MINJUMP_DISTANCE		15
#define GIVEUP_TIME				2000

#define MAX_ROAM_SPEED			0.05f
#define MAX_CHASE_SPEED			0.8f
#define HIGH_MANOEUVRE_DISTANCE	5

void CMonster::Update(float *PlayerPos, float *LightPos)
{
	MonsterStates OldState = CurrentState;

	StateTimer++;
	/* get (squared) distance from player */
	float Diff[3] = {Pos[0] - PlayerPos[0], Pos[1] - PlayerPos[1], Pos[2] - PlayerPos[2]};
	float SqDist = Diff[0]*Diff[0] + Diff[1]*Diff[1] + Diff[2]*Diff[2];

	/* determine a new state maybe */
	switch(CurrentState)
	{
		case MS_CHASING:	/* charge towards !! */
			// did player escape?
			if(SqDist > GIVEUP_DISTANCE*GIVEUP_DISTANCE)
			{
				CurrentState = MS_ROAMING;
			}
			else
			{
				if(SqDist < HIGH_MANOEUVRE_DISTANCE*HIGH_MANOEUVRE_DISTANCE)
				{
					Vec[0] *= 0.98f;
					Vec[1] *= 0.98f;
					Vec[2] *= 0.98f;
					if(PlayerPos[0] < Pos[0]) Vec[0] -= 0.004f; else Vec[0] += 0.004f;
					if(PlayerPos[1] < Pos[1]) Vec[1] -= 0.004f; else Vec[1] += 0.004f;
					if(PlayerPos[2] < Pos[2]) Vec[2] -= 0.004f; else Vec[2] += 0.004f;
				}
				else
				{
					if(PlayerPos[0] < Pos[0]) Vec[0] -= 0.001f; else Vec[0] += 0.001f;
					if(PlayerPos[1] < Pos[1]) Vec[1] -= 0.001f; else Vec[1] += 0.001f;
					if(PlayerPos[2] < Pos[2]) Vec[2] -= 0.001f; else Vec[2] += 0.001f;
				}

				if(!(StateTimer&127) && (SqDist > MINJUMP_DISTANCE*MINJUMP_DISTANCE) && fabs(Vec[2]) < 0.02f && Pos[2] < 4) Vec[2] += 0.08f;
			}

			/* max speed clip */
			if(Vec[0] < -MAX_CHASE_SPEED) Vec[0] = -MAX_CHASE_SPEED;
			if(Vec[0] > MAX_CHASE_SPEED) Vec[0] = MAX_CHASE_SPEED;
			if(Vec[1] < -MAX_CHASE_SPEED) Vec[1] = -MAX_CHASE_SPEED;
			if(Vec[1] > MAX_CHASE_SPEED) Vec[1] = MAX_CHASE_SPEED;
			if(Vec[2] < -MAX_CHASE_SPEED) Vec[2] = -MAX_CHASE_SPEED;
			if(Vec[2] > MAX_CHASE_SPEED) Vec[2] = MAX_CHASE_SPEED;

			/* tired? */
			if(StateTimer == GIVEUP_TIME)
				CurrentState = MS_RESTING;
		break;
		case MS_RESTING:
			if(StateTimer == GIVEUP_TIME)
				CurrentState = MS_ROAMING;
		break;
		case MS_CANSMELL:
			// if can smell, just do nothing in the hope that the smell continues
		
			// player now so far away that smell is gone?
			if(SqDist > SMELL_DISTANCEHIGH*SMELL_DISTANCEHIGH)
				CurrentState = MS_ROAMING;

			// been able to smell long enough to go into a charge?
			if(StateTimer == SMELL_TIME)
				CurrentState = MS_CHASING;

			if(Target[0] < Pos[0]) Vec[0] -= 0.001f; else Vec[0] += 0.001f;
			if(Target[1] < Pos[1]) Vec[1] -= 0.001f; else Vec[1] += 0.001f;
			if(Target[2] < Pos[2]) Vec[2] -= 0.001f; else Vec[2] += 0.001f;

			/* max speed clip */
			if(Vec[0] < -MAX_ROAM_SPEED) Vec[0] = -MAX_ROAM_SPEED;
			if(Vec[0] > MAX_ROAM_SPEED) Vec[0] = MAX_ROAM_SPEED;
			if(Vec[1] < -MAX_ROAM_SPEED) Vec[1] = -MAX_ROAM_SPEED;
			if(Vec[1] > MAX_ROAM_SPEED) Vec[1] = MAX_ROAM_SPEED;
			if(Vec[2] < -MAX_ROAM_SPEED) Vec[2] = -MAX_ROAM_SPEED;
			if(Vec[2] > MAX_ROAM_SPEED) Vec[2] = MAX_ROAM_SPEED;
		break;
		default:
			/* if closer than 10 units, "smell" player */
			if(SqDist < SMELL_DISTANCELOW*SMELL_DISTANCELOW)
			{
				CurrentState = MS_CANSMELL;
				Target[0] = (Pos[0] + PlayerPos[0]) * 0.5f;
				Target[1] = (Pos[1] + PlayerPos[1]) * 0.5f;
				Target[2] = (Pos[2] + PlayerPos[2]) * 0.5f;
			}
			else
			{
				/* time to pick a new random point? */
				if(!(StateTimer&63))
					GetNewTarget(LightPos);

				if(Target[0] < Pos[0]) Vec[0] -= 0.001f; else Vec[0] += 0.001f;
				if(Target[1] < Pos[1]) Vec[1] -= 0.001f; else Vec[1] += 0.001f;
				if(Target[2] < Pos[2]) Vec[2] -= 0.001f; else Vec[2] += 0.001f;

				if(!(StateTimer&255) && (SqDist > MINJUMP_DISTANCE*MINJUMP_DISTANCE) && fabs(Vec[2]) < 0.02f && Pos[2] < 4) Vec[2] += 0.08f;

				/* max speed clip */
				if(Vec[0] < -MAX_ROAM_SPEED) Vec[0] = -MAX_ROAM_SPEED;
				if(Vec[0] > MAX_ROAM_SPEED) Vec[0] = MAX_ROAM_SPEED;
				if(Vec[1] < -MAX_ROAM_SPEED) Vec[1] = -MAX_ROAM_SPEED;
				if(Vec[1] > MAX_ROAM_SPEED) Vec[1] = MAX_ROAM_SPEED;
				if(Vec[2] < -MAX_ROAM_SPEED) Vec[2] = -MAX_ROAM_SPEED;
				if(Vec[2] > MAX_ROAM_SPEED) Vec[2] = MAX_ROAM_SPEED;
			}
		break;
	}

	OnGround = DoMovement(Pos, Vec, Size);
		
	if(OldState != CurrentState)
	{
		StateChangedFlag = true;
		StateTimer = 0;
		
		if(CurrentState == MS_CHASING || OldState == MS_CHASING)
			Shape.FlipNormals();
	}
}

bool CMonster::StateChanged()
{
	if(StateChangedFlag)
	{
		StateChangedFlag = false;
		return true;
	}
	return false;
}

MonsterStates CMonster::State()
{
	return CurrentState;
}

float *CMonster::GetPos()
{
	return Pos;
}

void CMonster::Draw()
{
	float RedDiffuse[4] = {1, 0, 0, 1}, RedAmbient[4] = {0.2, 0, 0, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, RedDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, RedAmbient);
	glPushMatrix();
		glTranslatef(Pos[0], Pos[1], -Pos[2]);
		Shape.Draw();
	glPopMatrix();
}

void CMonster::ProjectShadow(float *LightPos)
{
	DrawSphereShadowVolume(LightPos, Pos);
}
