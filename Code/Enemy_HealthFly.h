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

	HealthFly(sf::Vector2i &pos, int level, 
		sf::Vertex *p_quad );
	void HandleQuery(QuadTreeCollider * qtc);
	void SetLevel(int lev);
	void AddToWorldTrees();
	sf::FloatRect GetAABB();

	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ClearSprite();
	void ResetEnemy();
	bool IsCollectible(); //depending on type of fly
	bool Collect();

	Tileset *ts;
	sf::Vertex *quad;
	int GetHealAmount();
};

#endif