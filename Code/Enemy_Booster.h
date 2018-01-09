#ifndef __ENEMY_BOOSTER_H__
#define __ENEMY_BOOSTER_H__

#include "Enemy.h"

struct Booster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		REFRESH,
		Count
	};

	Action action;
	Booster(GameSession *owner,
		sf::Vector2i &pos, int strength);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	//CollisionBox hurtBody;
	CollisionBody *hitBody;

	int strength;
	//HitboxInfo *hitboxInfo;

	int animationFactor;
};

#endif