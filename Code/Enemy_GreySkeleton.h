#ifndef __ENEMY_GREYSKELETON_H__
#define __ENEMY_GREYSKELETON_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "PlayerComboer.h"

struct FinalSkeletonPostFightScene;

struct GreySkeleton : Enemy
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

	PlayerComboer playerComboer;
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

	FinalSkeletonPostFightScene *postFightScene;

	GreySkeleton(ActorParams *ap);

	~GreySkeleton();
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
	void UpdatePreFrameCalculations();
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

#endif