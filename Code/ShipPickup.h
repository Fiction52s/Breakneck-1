#ifndef __SHIPPICKUP_H__
#define __SHIPPICKUP_H__

#include "Enemy.h"

struct ShipPickup : Enemy
{
	enum Action
	{
		IDLE,
		FOUND,
		DONE,
		Count
	};

	ShipPickup(ActorParams *ap);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);

	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;

};

#endif