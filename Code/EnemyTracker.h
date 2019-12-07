#ifndef __ENEMYTRACKER_H__
#define __ENEMYTRACKER_H__

#include "VectorMath.h"

struct GameSession;
struct Enemy;
struct EnemyTracker
{
	bool GetClosestEnemyPos( GameSession *owner, 
		V2d &pos,
		double radius,
		Enemy *&foundEnemy,
		int &foundIndex );
	virtual bool IsValidEnemy( Enemy *e );
};

#endif