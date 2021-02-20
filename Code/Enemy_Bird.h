#ifndef __ENEMY_BIRD_H__
#define __ENEMY_BIRD_H__

#include "Bullet.h"
#include "Movement.h"
#include "Enemy_BirdShuriken.h"
#include "Boss.h"

struct BirdPostFightScene;
struct BirdPostFight2Scene;
struct BirdPostFight3Scene;

struct Bat;



struct Bird : Summoner, Boss
{
	enum Action
	{
		WAIT,
		COMBOMOVE,
		PUNCH,
		KICK,
		MOVE_NODE_LINEAR,
		MOVE_NODE_QUADRATIC,
		MOVE_CHASE,
		RUSH,
		MOVE,
		SHURIKEN_SHOTGUN,
		UNDODGEABLE_SHURIKEN,
		SUMMON,
		SEQ_WAIT,
		A_Count
	};

	SummonGroup batSummonGroup;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	BirdPostFightScene * postFightScene;
	BirdPostFight2Scene *postFightScene2;
	BirdPostFight3Scene *postFightScene3;

	BirdShurikenPool shurPool;
	
	int fireCounter;

	

	//BirdCommand actionQueue[3];
	//int actionQueueIndex;

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;
	Tileset *ts_bulletExplode;

	Bird(ActorParams *ap);
	~Bird();

	//Summoner functions
	void InitEnemyForSummon(SummonGroup *group,
		Enemy *e);

	//Enemy functions
	int SetLaunchersStartIndex(int ind);
	void DirectKill();
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();

	//Boss functions
	bool TryComboMove(V2d &comboPos, int comboMoveDuration,
		int moveDurationBeforeStartNextAction,
		V2d &comboOffset );
	int ChooseActionAfterStageChange();
	void ActivatePostFightScene();
	void ActionEnded();
	void HandleAction();
	void StartAction();
	void SetupPostFightScenes();
	void SetupNodeVectors();
	bool IsDecisionValid(int d);
	bool IsEnemyMoverAction( int a);

	//My functions
	void Wait(int numFrames);
	void SequenceWait();
	void StartFight();
	void LoadParams();
	
	//Rollback functions
	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif