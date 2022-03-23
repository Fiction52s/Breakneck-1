#ifndef __ENEMY_GREYSKELETON_H__
#define __ENEMY_GREYSKELETON_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_Thorn.h"

struct FinalSkeletonPostFightScene;
struct ThornPool;


struct GreySkeleton : Boss
{
	enum Action
	{
		WAIT,
		COMBOMOVE,
		MOVE,
		THORN_TEST,
		SEQ_WAIT,
		A_Count
	};

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	Tileset *ts_move;

	FinalSkeletonPostFightScene *postFightScene;

	ThornPool thornPool;

	GreySkeleton(ActorParams *ap);

	~GreySkeleton();

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	int GetNumSimulationFramesRequired();

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