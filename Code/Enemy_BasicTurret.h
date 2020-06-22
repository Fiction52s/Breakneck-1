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
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	sf::SoundBuffer *fireSound;
	CollisionBox prelimBox[3];

	void ResetEnemy();
	Tileset *ts;
	Tileset *ts_aura;
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