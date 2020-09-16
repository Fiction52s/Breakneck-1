#ifndef __ENEMY_DIMENSION_EYE_H__
#define __ENEMY_DIMENSION_EYE_H__

#include "Enemy.h"
#include "Movement.h"

struct GameSession;

struct DimensionEye : Enemy
{
	enum Action
	{
		IDLE,
		DIE,
		A_Count
	};

	Tileset *ts;
	void Setup();
	GameSession *myBonus;

	DimensionEye(ActorParams *ap);
	void ProcessState();
	void HandleHitAndSurvive();

	void EnemyDraw(sf::RenderTarget *target);
	void ProcessHit();
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
};

#endif