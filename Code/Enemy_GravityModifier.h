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
	GravityModifier(GameSession *owner,
		sf::Vector2i &pos, int level,
		bool increaser);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	bool Modify();
	bool IsModifiable();

	sf::Sprite sprite;
	Tileset *ts;
	Tileset *ts_refresh;

	double gravFactor;
	int duration;

	int actionLength[Count];
	int animFactor[Count];
};

#endif