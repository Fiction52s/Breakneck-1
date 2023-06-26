#ifndef __ENEMY_BASICTURRET_H__
#define __ENEMY_BASICTURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct Shield;



struct BasicTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
		A_Count
	};

	const static int maxBullets = 16;

	struct MyData : StoredEnemyData
	{
	};

	MyData data;

	Tileset *ts;
	Tileset *ts_bulletExplode;
	HitboxInfo *bulletHitboxInfo;
	CollisionBody *prelimBody;
	CollisionBox prelimBox;
	double detectRad;
	int frameTestCounter;
	SoundInfo *fireSound;
	int animationFactor;
	double bulletSpeed;
	Shield *testShield;	
	int framesWait;
	

	BasicTurret(ActorParams *ap);
	~BasicTurret();
	void SetLevel(int lev);
	void UpdateOnPlacement(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void Setup();
	void UpdatePreLauncherPhysics();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer( int playerIndex,
		BasicBullet *b,
		int hitResult );
	void ResetEnemy();
	void DirectKill();
	void SetupPreCollision();
	V2d SetupPrelimBox();
	
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif