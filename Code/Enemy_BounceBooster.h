#ifndef __ENEMY_BOUNCEBOOSTER_H__
#define __ENEMY_BOUNCEBOOSTER_H__

#include "Enemy.h"
#include "Enemy_Booster.h"

struct BounceBooster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		REFRESH,
		Count
	};

	Action action;
	BounceBooster(GameSession *owner,
		sf::Vector2i &pos, bool upOnly,
		int level);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	bool Boost();
	bool IsBoostable();

	sf::Sprite sprite;
	Tileset *ts;
	Tileset *ts_refresh;

	int strength;
	bool upOnly;



	int actionLength[Count];
	int animFactor[Count];
};

#endif