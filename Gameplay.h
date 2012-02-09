#ifndef __GAMEPLAY_H
#define __GAMEPLAY_H

#include "ebgf.h"
#include "Monster.h"

#define LIGHT_WIDTH		18

extern int LastScore, HighScore;

#define SOUND_PING			0
#define SOUND_COLLECT		1
#define SOUND_THROW			3
#define SOUND_JUMP			4
#define SOUND_GAMEOVER		5
#define SOUND_WARNING		6
extern void TriggerSound(int id);
extern void SetDroneVolume(float);

class CGameplay: public CGameScreen
{
	public:
		void Message(EBGF_Message &Message);

	private:
		/* player state */
		float PlayerAngle[2];
		float PlayerPos[3];
		float PlayerVelocity[3];
		float PlayerSize[3];

		/* light state */
		float LightPos[4], LightVelocity[3], LightSize[3];

		/* next mineral */
		float MineralPos[3];
		int CollectedMinerals;

		/* for placing mineral in a new spot */
		void RePlaceMineral();
		float DistanceToMineral();
		bool PlayerCanReach(int x, int y, int z);
		bool PlayerCanReachR(int x, int y, int z);
		Uint8 CheckMap[128][128];

		/* level shadow caster */
		void CastLevel(float *TLightPos);

		/* enemy! Very scary! */
		CMonster *Monster;
		int MSGPointer;
		Uint32 MSGStartTime;
		bool MonsterVeryClose;
};

#endif
