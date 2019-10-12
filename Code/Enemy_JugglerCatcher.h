#ifndef __ENEMY_JUGGLERCATCHER_H__
#define __ENEMY_JUGGLERCATCHER_H__

#include "Enemy.h"

struct JugglerCatcher : Enemy
{
	enum Action
	{
		NEUTRAL,
		CATCH,
		DEATH,
		Count
	};

	Action action;
	JugglerCatcher(GameSession *owner,
		sf::Vector2i &pos, int level);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	bool CanCatch();
	bool Catch();

	int actionLength[Count];
	int animFactor[Count];
};

#endif