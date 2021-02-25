#ifndef __ENEMY_GATOR_H__
#define __ENEMY_GATOR_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_GatorWaterOrb.h"

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
		COMBOMOVE,
		MOVE,
		SUMMON,
		SEQ_WAIT,
		A_Count
	};

	NodeGroup nodeGroupA;

	GatorWaterOrbPool orbPool;

	SummonGroup swarmSummonGroup;

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	GatorPostFightScene *postFightScene;

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