#ifndef __ENEMY_GRAVITYMODIFIER_H__
#define __ENEMY_GRAVITYMODIFIER_H__

#include "Enemy.h"

struct GravityModifier : Enemy
{
	enum Action
	{
		NEUTRAL,
		MODIFY,
		REFRESH,
		Count
	};

	Action action;
	GravityModifier(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	//void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Modify();
	bool IsModifiable();

	sf::Sprite sprite;
	Tileset *ts;
	Tileset *ts_refresh;

	double gravFactor;

	bool increaser;
	int duration;

	int actionLength[Count];
	int animFactor[Count];
};

#endif