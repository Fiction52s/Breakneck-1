#ifndef __GROUNDED_WARPER_H__
#define __GROUNDED_WARPER_H__

#include "Enemy.h"
#include "Movement.h"

struct GameSession;

struct GroundedWarper : Enemy
{
	enum Action
	{
		ACTIVE,
		APPEAR,
		DISAPPEAR,
		HIDDEN,
		A_Count
	};

	Tileset *ts;
	void Setup();
	GameSession *myBonus;

	GroundedWarper(ActorParams *ap);
	void ProcessState();
	void HandleHitAndSurvive();

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	bool CheckHitPlayer(int index);
};

#endif