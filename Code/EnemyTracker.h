#ifndef __ENEMYTRACKER_H__
#define __ENEMYTRACKER_H__

#include "VectorMath.h"

struct GameSession;
struct Enemy;
struct EnemyTracker
{
	bool GetClosestEnemyPos(
		V2d &pos,
		double radius,
		Enemy *&foundEnemy,
		int &foundIndex );
	virtual bool IsValidTrackEnemy( Enemy *e );
};

#endif