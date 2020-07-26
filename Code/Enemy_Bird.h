#ifndef __ENEMY_BIRD_H__
#define __ENEMY_BIRD_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"

struct Bird : Enemy, LauncherEnemy
{
	enum Action
	{
		PUNCH,
		FOLLOWUPPUNCH,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int fireCounter;

	Tileset *ts_bulletExplode;
	Tileset *ts_aura;
	int animationFactor;
	int bulletSpeed;
	int framesBetween;

	bool hitPlayer;
	bool predict;

	int targetPlayerIndex;
	LineMovement *move;
	MovementSequence ms;

	HitboxInfo punchHitboxInfo;
	HitboxInfo kickHitboxInfo;

	Bird(ActorParams *ap);

	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);
	void UpdatePreFrameCalculations();
	void ProcessState();
	void CalcTargetAfterHit();
	void UpdateHitboxes();

	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	Tileset *ts_punch;

	V2d targetPos;
	int framesToArrive;
};

#endif