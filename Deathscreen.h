#ifndef __DEATHSCREEN_H
#define __DEATHSCREEN_H

#include "ebgf.h"

class CDeathScreen: public CGameScreen
{
	public:
		void Message(EBGF_Message &Message);
};

#endif
