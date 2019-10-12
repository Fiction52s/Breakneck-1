#ifndef __ENEMY_HEALTHFLY_H__
#define __ENEMY_HEALTHFLY_H__

#include "Enemy.h"

struct HealthFly : Enemy
{
	enum Action
	{
		NEUTRAL,
		DEATH,
		Count
	};

	Action action;
	HealthFly(GameSession *owner,
		sf::Vector2i &pos, int level, int index);
	void HandleQuery(QuadTreeCollider * qtc);

	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ClearSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	bool CanCollect(); //depending on type of fly
	bool Collect();

	//sf::Sprite sprite;
	Tileset *ts;

	int flyIndex;
	sf::Vertex *va;

	int healAmount;

	int actionLength[Count];
	int animFactor[Count];
};

#endif