#ifndef __ENEMY_BOUNCEFLOATER_H__
#define __ENEMY_BOUNCEFLOATER_H__

#include "Enemy.h"

struct ComboObject;

struct BounceFloater : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_BOUNCE,
		S_RECOVER,
		S_Count
	};

	BounceFloater(ActorParams *ap);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void SetLevel(int lev);

	Tileset *ts;
};

#endif