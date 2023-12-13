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

	//Rollback
	struct MyData : StoredEnemyData
	{
	};

	Tileset *ts;
	MyData data;
	V2d pickupPos;

	ShipPickup(ActorParams *ap);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateSpriteFromParams(ActorParams *ap);

	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif