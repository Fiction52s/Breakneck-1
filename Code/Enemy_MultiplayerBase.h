#ifndef __ENEMY_MULTIPLAYERBASE_H__
#define __ENEMY_MULTIPLAYERBASE_H__

#include "Enemy.h"

struct MultiplayerBase : Enemy
{
	enum Action
	{
		S_FLOAT,
		//S_EXPLODE,
		S_Count
	};

	Tileset *ts;
	int actorIndex;

	MultiplayerBase(ActorParams * ap);
	~MultiplayerBase();

	void ProcessHit();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	
};

#endif