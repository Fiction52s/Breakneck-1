#ifndef __ENEMY_TIGER_H__
#define __ENEMY_TIGER_H__

#include "Boss.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_TigerGrindBullet.h"
#include "Enemy_TigerSpinTurret.h"
//#include "Enemy_TigerTarget.h"

struct TigerPostFightScene;
struct TigerPostFight2Scene;

struct Tiger : Boss, LauncherEnemy, Summoner,
	RayCastHandler
{
	enum Action
	{
		WAIT,
		GATHER_ENERGY_START,
		GATHER_ENERGY_LOOP,
		GATHER_ENERGY_END,
		START_GRIND,
		MOVE_GRIND,
		MOVE_JUMP,
		JUMP_LAND,
		JUMP_SQUAT,
		MOVE_LUNGE,
		MOVE_RUSH,
		TEST,
		COMBOMOVE,
		SUMMON,
		THROW_SPINTURRET,
		SEQ_WAIT,
		SUMMON_FLAME_TARGETS,
		LAUNCH_FLAME_TARGETS,
		FLAME_STUN,
		TEST_POST,
		A_Count
	};

	GameSession *myBonus;

	SummonGroup palmSummonGroup;

	SummonGroup targetGroup;

	TigerPostFightScene *postFightScene;
	TigerPostFight2Scene *postFightScene2;

	int moveOnlyFrames;
	int moveOnlyMaxFrames;
	int defaultMoveOnlyFrames;

	Tileset *ts_move;
	Tileset *ts_grind;
	Tileset *ts_roar;

	//SummonGroup spinTurretSummonGroup;

	TigerGrindBulletPool snakePool;

	bool ignorePointsCloserThanPlayer;
	double playerDist;

	int framesSinceRush;
	//TigerSpinTurretPool spinPool;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;
	NodeGroup nodeGroupC;
	NodeGroup nodeGroupGrind;


	int numFlamesHitBy;
	int currMaxTargets;


	V2d lastTargetDestroyedPos;

	Tiger(ActorParams *ap);
	~Tiger();

	//Summoner functions
	void InitEnemyForSummon(SummonGroup *group,
		Enemy *e);

	//Launcher functions

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleRayCollision(Edge *edge, double edgeQuantity,
		double rayPortion);
	void FrameIncrement();
	void DrawMinimap(sf::RenderTarget * target);
	int GetNumSimulationFramesRequired();
	int ChooseActionAfterStageChange();
	void AddToGame();

	//Boss functions
	bool TryComboMove(V2d &comboPos, int comboMoveDuration,
		int moveDurationBeforeStartNextAction,
		V2d &comboOffset);
	void HandleSummonedChildRemoval(Enemy *e);
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
	void HitWithFlame();

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