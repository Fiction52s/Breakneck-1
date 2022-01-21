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

	

	BasicTurret(ActorParams *ap);
	~BasicTurret();
	void SetLevel(int lev);
	void UpdateOnPlacement(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void Setup();
	Tileset *ts_bulletExplode;
	int frameTestCounter;
	void UpdatePreLauncherPhysics();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer( int playerIndex,
		BasicBullet *b,
		int hitResult );
	SoundInfo *fireSound;
	CollisionBox prelimBox[3];

	void ResetEnemy();
	Tileset *ts;
	double detectRad;

	const static int maxBullets = 16;

	void DirectKill();

	Shield *testShield;

	HitboxInfo *bulletHitboxInfo;

	int framesWait;
	int firingCounter;

	void SetupPreCollision();
	CollisionBody *prelimBody;

	int animationFactor;
	double bulletSpeed;
};

#endif