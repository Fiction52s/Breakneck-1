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
		PUNCH2,
		KICK,
		MOVE_NODE_LINEAR,
		MOVE_NODE_QUADRATIC,
		MOVE_CHASE,
		//MOVE_NODE_LINEAR_BASIC_SHURIKEN,
		//MOVE_NODE_QUADRATIC_BASIC_SHURIKEN,
		RUSH,
		MOVE,
		SHURIKEN_SHOTGUN_1,
		SHURIKEN_SHOTGUN_2,
		BASIC_SHURIKEN,
		UNDODGEABLE_SHURIKEN,
		SUMMON,
		SEQ_WAIT,
		TEST_POST,
		GATHER_ENERGY_START,
		GATHER_ENERGY_LOOP,
		GATHER_ENERGY_END,
		A_Count
	};

	SummonGroup batSummonGroup;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	RandomPicker attackPicker;

	GameSession *myBonus;

	std::vector<int> waitFrames;
	std::vector<int> chargeFrames;

	BirdPostFightScene * postFightScene;
	BirdPostFight2Scene *postFightScene2;
	BirdPostFight3Scene *postFightScene3;

	BirdShurikenPool shurPool;
	
	int fireCounter;

	Tileset *ts_punch;
	Tileset *ts_punch2;
	Tileset *ts_kick;
	Tileset *ts_idle;
	Tileset *ts_bulletExplode;
	Tileset *ts_charge;
	Tileset *ts_throw;

	int summonStartStage;
	int shotgunStartStage;
	int chaseStartStage;

	int idleCounter;
	int idleAnimFactor;

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
		int moveDurationBeforeStartNextAction, int framesRemaining,
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