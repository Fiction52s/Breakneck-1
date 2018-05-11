#ifndef __SHIPPICKUP_H__
#define __SHIPPICKUP_H__

#include "Enemy.h"
struct ShipPickup : Enemy
{
	ShipPickup( GameSession *owner,
		Edge *ground, double quantity,
		bool facingRight );
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);


	void ResetEnemy();

	bool facingRight;
	sf::Sprite sprite;
	Tileset *ts;
	Edge *ground;
	double edgeQuantity;
};

#endif