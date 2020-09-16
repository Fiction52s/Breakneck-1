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

	DimensionExit(ActorParams *ap);
	void ProcessState();
	void HandleNoHealth();

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
};

#endif