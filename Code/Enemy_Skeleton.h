#ifndef __ENEMY_SKELETON_H__
#define __ENEMY_SKELETON_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"
#include "Enemy_GatorWaterOrb.h"

struct Skeleton : Enemy, RayCastHandler
{
	enum Action
	{
		COMBOMOVE,
		MOVE,
		WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int moveFrames;
	int waitFrames;

	std::string nodeAStr;
	std::string nodeBStr;

	GatorWaterOrbPool orbPool;

	PlayerComboer playerComboer;
	EnemyMover enemyMover;

	Tileset *ts_bulletExplode;
	Tileset *ts_aura;
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

	V2d rayEnd;
	V2d rayStart;
	Edge *rcEdge;
	double rcQuantity;

	Skeleton(ActorParams *ap);

	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void SetCommand(int index, BirdCommand &bc);
	void UpdatePreFrameCalculations();
	void ProcessState();
	void UpdateHitboxes();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);
};

#endif