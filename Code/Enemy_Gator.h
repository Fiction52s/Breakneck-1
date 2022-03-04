#ifndef __ENEMY_GATOR_H__
#define __ENEMY_GATOR_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_GatorWaterOrb.h"
#include "Enemy_GatorSuperOrb.h"
#include "BonusHandler.h"

struct GatorPostFightScene;

struct Gator : Boss, Summoner, BonusHandler,
	RayCastHandler
{
	//DOMINATION
	enum Action
	{
		WAIT,
		MOVE_NODE_LINEAR,
		MOVE_NODE_QUADRATIC,
		MOVE_CHASE,
		ATTACK,
		COMBOMOVE,
		MOVE,
		SUMMON,
		MOVE_WANTS_TO_BITE,
		MOVE_WANTS_TO_TRIPLE_RUSH,
		MOVE_TO_ORB_ATTACK_1,
		MOVE_TO_ORB_ATTACK_2,
		MOVE_TO_ORB_ATTACK_3,
		BITE_ATTACK,
		BITE_STUCK,
		BITE_RECOVER,
		TRIPLE_LUNGE_1,
		TRIPLE_LUNGE_WAIT_1,
		TRIPLE_LUNGE_2,
		TRIPLE_LUNGE_WAIT_2,
		TRIPLE_LUNGE_3,
		REDIRECT_ORBS,
		TEST_ORBS,
		ORB_ATTACK_1,
		ORB_ATTACK_2,
		ORB_ATTACK_3,
		SEQ_WAIT,
		TEST_POST,
		TIME_ORB_ATTACK,
		CHASE_ATTACK,
		A_Count
	};

	enum MoveMode
	{
		MM_STOP,
		MM_APPROACH,
		MM_FLEE,
		MM_RANDOM,
	};

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;
	
	int moveMode;

	std::vector<V2d> nodePosAVec;
	std::vector<V2d> nodePosBVec;
	const static int NUM_ORB_POOLS = 3;
	GatorWaterOrbPool orbPool[NUM_ORB_POOLS];
	GatorWaterOrbPool timeOrbPool;
	GatorSuperOrbPool superOrbPool;

	RandomPicker orbTypePicker;

	SummonGroup swarmSummonGroup;

	bool redirectingOrbs;
	int redirectFrame;
	int redirectRate;

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;
	Tileset *ts_bite;

	double currMoveSpeed;

	GatorPostFightScene *postFightScene;

	V2d biteRushDir;

	GameSession *myBonus;

	V2d oldPlayerPos;

	double orbAttack1Angle;

	Gator(ActorParams *ap);
	~Gator();

	//summoner functions
	void InitEnemyForSummon(SummonGroup *group,
		Enemy *e);

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
	int GetNumSimulationFramesRequired();
	//void HandleRemove();

	//Boss functions
	bool TryComboMove(V2d &comboPos, int comboMoveDuration,
		int moveDurationBeforeStartNextAction,
		int framesRemaining,
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

	void MoveRandomly();
	void MoveTowardsPlayer();
	void MoveAwayFromPlayer();

	void ThrowTimeOrbTowardsPlayer();
	void ThrowTimeOrbRandomly();

	void SortNodePosAVec( V2d &sortPos );
	void SortNodePosBVec(V2d &sortPos);
	void UpdateMove();

	void OrbAttack1();
	void OrbAttack1_1();
	void OrbAttack1_2();
	void OrbAttack1_3();


	void OrbAttack2();

	void OrbAttack3();

	void OrbAttack4();

	void OrbAttack5();

	void TimeOrbAttack1();

	void InitBonus();

	//Rollback
	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
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