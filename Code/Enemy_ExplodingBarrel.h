#ifndef __ENEMY_EXPLODINGBARREL_H__
#define __ENEMY_EXPLODINGBARREL_H__

#include "Enemy.h"

struct ComboObject;

struct ExplodingBarrel : Enemy
{
	enum Action
	{
		S_IDLE,
		S_TINYCHARGE,
		S_CHARGE,
		S_ABOUT_TO_EXPLODE,
		S_EXPLODE,
		S_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	sf::CircleShape testCircle;
	Tileset *ts;
	CollisionBody explosion;
	HitboxInfo explosionInfo;

	ExplodingBarrel(ActorParams * ap);
	~ExplodingBarrel();

	void SetLevel(int lev);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void DirectKill();
	void HandleNoHealth();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif