#ifndef __ENEMY_TIGER_SPINTURRET_H__
#define __ENEMY_TIGER_SPINTURRET_H__

#include "Enemy.h"
#include "Bullet.h"
#include <vector>

struct TigerSpinTurret;

//struct TigerSpinTurretPool
//{
//	TigerSpinTurretPool();
//	~TigerSpinTurretPool();
//	void Reset();
//	TigerSpinTurret *Throw(V2d &pos, V2d &dir );
//	void Draw(sf::RenderTarget *target);
//	std::vector<TigerSpinTurret*> turretVec;
//	sf::Vertex *verts;
//	Tileset *ts;
//	int numTurrets;
//};

struct TigerSpinTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		HOMING,
		TURRET,
		A_Count
	};

	int framesToLive;
	int maxFramesToLive;

	int bulletSpeed;
	int framesBetween;

	Tileset *ts_bulletExplode;

	int fireCounter;
	double speed;
	Tileset *ts;

	V2d velocity;

	V2d facingDir;

	V2d initVel;


	TigerSpinTurret(ActorParams *ap);
	bool IsHitFacingRight();
	void Throw(V2d &pos, V2d &dir );
	void Init(V2d &pos, V2d &dir);
	void SetLevel(int lev);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
};

#endif