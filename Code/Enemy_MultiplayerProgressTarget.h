#ifndef __ENEMY_MULTIPLAYERPROGRESSTARGET_H__
#define __ENEMY_MULTIPLAYERPROGRESSTARGET_H__

#include "Enemy.h"



struct MultiplayerProgressTarget : Enemy
{
	enum Action
	{
		S_FLOAT,
		//S_EXPLODE,
		S_Count
	};

	struct MyData
	{
		HitboxInfo *receivedHit;
		Enemy *prev;
		Enemy *next;
		Actor *receivedHitPlayer;
		int numHealth;
		int pauseFrames;
		bool dead;
		bool hitBy[4];
	};

	bool hitBy[4];
	Tileset *ts;
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBuffer(unsigned char *buf);

	HitboxInfo *IsHit(int pIndex);
	void ProcessHit();

	MultiplayerProgressTarget(ActorParams * ap);
	~MultiplayerProgressTarget();

	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
};

#endif