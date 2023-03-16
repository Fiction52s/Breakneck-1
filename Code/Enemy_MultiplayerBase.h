#ifndef __ENEMY_MULTIPLAYERBASE_H__
#define __ENEMY_MULTIPLAYERBASE_H__

#include "Enemy.h"



struct MultiplayerBase : Enemy
{
	enum Action
	{
		S_FLOAT,
		//S_EXPLODE,
		S_Count
	};

	struct MyData
	{
		/*HitboxInfo *receivedHit;
		Enemy *prev;
		Enemy *next;
		Actor *receivedHitPlayer;
		int numHealth;
		int pauseFrames;
		bool dead;*/
	};

	Tileset *ts;
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	HitboxInfo *IsHit(int pIndex);

	MultiplayerBase(ActorParams * ap);
	~MultiplayerBase();

	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
};

#endif