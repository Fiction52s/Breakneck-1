#ifndef __ENEMY_BIRD_H__
#define __ENEMY_BIRD_H__

#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
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

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	SummonGroup batSummonGroup;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	BirdPostFightScene * postFightScene;
	BirdPostFight2Scene *postFightScene2;
	BirdPostFight3Scene *postFightScene3;

	BirdShurikenPool shurPool;
	

	int fireCounter;

	Tileset *ts_bulletExplode;

	std::map<int, int> hitboxStartFrame;

	BirdCommand actionQueue[3];
	int actionQueueIndex;

	HitboxInfo hitboxInfos[A_Count];

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	V2d targetPos;
	int framesToArrive;

	Bird(ActorParams *ap);
	~Bird();

	int ChooseActionAfterStageChange();
	void ActivatePostFightScene();

	
	void StartAction();
	void SetupPostFightScenes();
	void SetupNodeVectors();

	bool IsDecisionValid(int d);

	bool IsEnemyMoverAction( int a);

	void Wait(int numFrames);
	
	
	void MoveToCombo();
	int SetLaunchersStartIndex(int ind);
	void SequenceWait();
	void StartFight();
	void LoadParams();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void SetCommand(int index, BirdCommand &bc);
	//void UpdatePreFrameCalculations();
	void ProcessState();
	void UpdateHitboxes();

	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);

	void HandleHitAndSurvive();
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();

	void SetHitboxInfo(int a);
	void InitEnemyForSummon(SummonGroup *group,
		Enemy *e);


};

#endif