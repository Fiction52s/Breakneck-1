#ifndef __ENEMY_MULTIPLAYERBASE_H__
#define __ENEMY_MULTIPLAYERBASE_H__

#include "Enemy.h"

struct BaseData
{
	HitboxInfo *receivedHit;
	Enemy *prev;
	Enemy *next;
	Actor *receivedHitPlayer;
	int numHealth;
	int pauseFrames;
	bool dead;
};

struct MultiplayerBase : Enemy
{
	enum Action
	{
		S_FLOAT,
		//S_EXPLODE,
		S_Count
	};

	Tileset *ts;
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBuffer(unsigned char *buf);

	MultiplayerBase(ActorParams * ap);
	~MultiplayerBase();

	//void ProcessHit();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	
};

#endif