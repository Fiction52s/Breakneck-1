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
	JugglerCatcher(GameSession *owner, bool hasMon,
		sf::Vector2i &pos, int level);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void ProcessHit();
	bool CanCatch();
	void Catch();

	sf::Sprite sprite;
	Tileset *ts;

	int actionLength[Count];
	int animFactor[Count];
};

#endif