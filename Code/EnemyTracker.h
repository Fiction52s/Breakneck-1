#ifndef __ENEMYTRACKER_H__
#define __ENEMYTRACKER_H__

#include "VectorMath.h"

struct GameSession;
struct Enemy;


enum TrackingType
{
	TRACKING_PLAYER_HOMING,
	TRACKING_PLAYER_HOMING_POWER,
	TRACKING_ENEMY,
	TRACKING_WIRE_TO_ENEMY,
	TRACKING_Count
};

struct EnemyTracker
{
	bool GetClosestEnemyPos( int trackingType,
		V2d &pos,
		double radius,
		Enemy *&foundEnemy,
		int &foundIndex );
	virtual bool CheckIfEnemyIsTrackable( Enemy *e, int trackingType );
};

#endif