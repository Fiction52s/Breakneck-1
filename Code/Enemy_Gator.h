#ifndef __ENEMY_GATOR_H__
#define __ENEMY_GATOR_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_GatorWaterOrb.h"
#include "Enemy_GatorSuperOrb.h"

struct GatorPostFightScene;

struct Gator : Boss, Summoner
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
		TRIPLE_LUNGE_1,
		TRIPLE_LUNGE_WAIT_1,
		TRIPLE_LUNGE_2,
		TRIPLE_LUNGE_WAIT_2,
		TRIPLE_LUNGE_3,
		REDIRECT_ORBS,
		SEQ_WAIT,
		SUPER_ORB,
		SEQ_RETRACT_SUPER_ORB,
		SEQ_HOLD_SUPER_ORB,
		A_Count
	};

	NodeGroup nodeGroupA;

	GatorWaterOrbPool orbPool;
	GatorSuperOrbPool superOrbPool;

	RandomPicker orbTypePicker;

	SummonGroup swarmSummonGroup;

	bool redirectingOrbs;
	int redirectFrame;
	int redirectRate;

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	GatorPostFightScene *postFightScene;

	GameSession *myBonus;

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
	void Setup();
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