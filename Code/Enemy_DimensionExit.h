#ifndef __ENEMY_DIMENSION_EXIT_H__
#define __ENEMY_DIMENSION_EXIT_H__

#include "Enemy.h"
#include "Movement.h"

struct GameSession;

struct DimensionExit : Enemy
{
	enum Action
	{
		IDLE,		
		A_Count
	};

	Tileset *ts;
	bool CountsForEnemyGate() { return false; }
	DimensionExit(ActorParams *ap);
	void ProcessState();
	void HandleNoHealth();
	void ProcessHit();
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
};

#endif