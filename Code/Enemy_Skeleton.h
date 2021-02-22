#ifndef __ENEMY_SKELETON_H__
#define __ENEMY_SKELETON_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_GatorWaterOrb.h"

struct SkeletonPostFightScene;
struct CoyoteHelper;

struct Skeleton : Boss, RayCastHandler
{
	enum Action
	{
		WAIT,
		MOVE_WIRE_DASH,
		MOVE_OTHER,
		COMBOMOVE,
		MOVE,
		SEQ_WAIT,
		A_Count
	};

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	SkeletonPostFightScene *postFightScene;
	CoyoteHelper *coyHelper;

	GatorWaterOrbPool orbPool;

	//CircleGroup testGroup;
	//CircleGroup testGroup2;

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	V2d rayEnd;
	V2d rayStart;
	Edge *rcEdge;
	double rcQuantity;
	bool ignorePointsCloserThanPlayer;
	double playerDist;

	Skeleton(ActorParams *ap);
	~Skeleton();

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	//Boss functions
	bool TryComboMove(V2d &comboPos, int comboMoveDuration,
		int moveDurationBeforeStartNextAction,
		V2d &comboOffset);
	int ChooseActionAfterStageChange();
	void ActivatePostFightScene();
	void ActionEnded();
	void HandleAction();
	void StartAction();
	void SetupPostFightScenes();
	void SetupNodeVectors();
	bool IsDecisionValid(int d);
	bool IsEnemyMoverAction(int a);

	//My functions
	void SeqWait();
	void StartFight();
	void LoadParams();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	

	

	//Rollback
	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif