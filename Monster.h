#ifndef __MONSTER_H
#define __MONSTER_H

#include "ConvexObject.h"

enum MonsterStates{MS_ROAMING, MS_SEENLIGHT, MS_CANSMELL, MS_CHASING, MS_RESTING};

class CMonster
{
	public:
		CMonster();

		void Update(float *PlayerPos, float *LightPos);
		bool StateChanged();
		MonsterStates State();
		float *GetPos();

		void Draw();
		void ProjectShadow(float *LightPos);

	private:
		float Pos[3], Vec[3], Size[3], Target[3];
		int SearchNum;
		bool StateChangedFlag;
		MonsterStates CurrentState;
		int StateTimer;
		bool OnGround;
		void GetNewTarget(float *LightPos);
		ConvexObject Shape;
};

#endif
