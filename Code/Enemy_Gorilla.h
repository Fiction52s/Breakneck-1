#ifndef __ENEMY_GORILLA_H__
#define __ENEMY_GORILLA_H__

#include "Enemy.h"
#include "Enemy_GorillaWall.h"

struct Gorilla : Enemy
{
	enum Action
	{
		IDLE,
		FOLLOW,
		ATTACK,
		RECOVER,
		//NEUTRAL,
		//WAKEUP,
		//ALIGN,
		//FOLLOW,
		//ATTACK,
		//RECOVER,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
	};
	MyData data;
	
	int createWallFrame;

	GorillaWallPool wallPool;

	Tileset *ts;

	double wallAmountCloser;

	Gorilla(ActorParams *ap);
	~Gorilla();

	void SetLevel(int lev);
	//void UpdateHitboxes();
	void ProcessState();
	void UpdateEnemyPhysics();

	void ActionEnded();

	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	void ResetEnemy();
	void AddToGame();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif