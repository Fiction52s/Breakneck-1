#ifndef __ENEMY_LASER_WOLF_H__
#define __ENEMY_LASER_WOLF_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Enemy_SkeletonLaser.h"

struct LaserWolf : Enemy//, LauncherEnemy
{
	enum Action
	{
		IDLE,
		ATTACK,
		RECOVER,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int fireCounter;
	};
	MyData data;

	SkeletonLaserPool laserPool;

	Tileset *ts;
	double maxSpeed;
	double accel;
	sf::CircleShape testCircle;
	//Shield *shield;
	Tileset *ts_bulletExplode;

	LaserWolf(ActorParams *ap);
	~LaserWolf();
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void FrameIncrement();
	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
	void AddToGame();

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);
	void DirectKill();


	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif