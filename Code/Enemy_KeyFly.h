#ifndef __ENEMY_KEYFLY_H__
#define __ENEMY_KEYFLY_H__

#include "Enemy.h"

struct KeyFly : Enemy
{
	enum Action
	{
		S_NEUTRAL,
		S_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;
	Tileset *ts_container;
	sf::Sprite containerSpr;

	KeyFly(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	bool CountsForEnemyGate() { return false; }
	void PlayDeathSound();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	//void ProcessHit();

	
};

#endif