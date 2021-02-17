#ifndef __ENEMY_GATOR_H__
#define __ENEMY_GATOR_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "Enemy_GatorWaterOrb.h"

struct GatorPostFightScene;

struct Gator : Enemy
{
	enum Action
	{
		COMBOMOVE,
		MOVE,
		WAIT,
		SEQ_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int moveFrames;
	int waitFrames;

	std::string nodeAStr;

	GatorWaterOrbPool orbPool;

	EnemyMover enemyMover;

	Tileset *ts_bulletExplode;
	int comboMoveFrames;

	int reachPointOnFrame[A_Count];

	bool hitPlayer;

	BirdCommand actionQueue[3];
	int actionQueueIndex;

	int targetPlayerIndex;

	HitboxInfo hitboxInfos[A_Count];

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;

	GatorPostFightScene *postFightScene;

	Gator(ActorParams *ap);

	~Gator();
	void Wait();
	void Setup();
	void ProcessHit();
	void StartFight();
	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void SetCommand(int index, BirdCommand &bc);
	void ProcessState();
	void UpdateHitboxes();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);
};

//struct Gator : Enemy, LauncherEnemy
//{
//	enum Action
//	{
//		DOMINATION,
//		A_Count
//	};
//
//	struct MyData : StoredEnemyData
//	{
//		int fireCounter;
//	};
//
//	int fireCounter;
//	
//	Tileset *ts;
//	Tileset *ts_bulletExplode;
//	Tileset *ts_aura;
//	int animationFactor;
//	int bulletSpeed;
//	int framesBetween;
//
//	Gator(ActorParams *ap);
//
//	int GetNumStoredBytes();
//	void StoreBytes(unsigned char *bytes);
//	void SetFromBytes(unsigned char *bytes);
//	void DirectKill();
//	void BulletHitTerrain(BasicBullet *b,
//		Edge *edge, V2d &pos);
//	void BulletHitPlayer(
//		int playerIndex, 
//		BasicBullet *b,
//		int hitResult);
//	void ProcessState();
//	
//	void EnemyDraw(sf::RenderTarget *target);
//	void HandleHitAndSurvive();
//
//	void IHitPlayer(int index = 0);
//	void UpdateSprite();
//	void ResetEnemy();
//	void UpdateEnemyPhysics();
//	void FrameIncrement();
//};

#endif