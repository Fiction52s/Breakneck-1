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
		MOVE_GRIND,
		MOVE_JUMP,
		MOVE_LUNGE,
		MOVE_RUSH,
		TEST,
		COMBOMOVE,
		SUMMON,
		THROW_SPINTURRET,
		SEQ_WAIT,
		A_Count
	};

	SummonGroup palmSummonGroup;

	SummonGroup targetGroup;

	TigerPostFightScene *postFightScene;
	TigerPostFight2Scene *postFightScene2;



	Tileset *ts_move;
	Tileset *ts_bulletExplode;

	SummonGroup spinTurretSummonGroup;

	TigerGrindBulletPool snakePool;

	bool ignorePointsCloserThanPlayer;
	double playerDist;

	int framesSinceRush;
	//TigerSpinTurretPool spinPool;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;
	NodeGroup nodeGroupC;

	V2d lastTargetDestroyedPos;

	Tiger(ActorParams *ap);
	~Tiger();

	//Summoner functions
	void InitEnemyForSummon(SummonGroup *group,
		Enemy *e);

	//Launcher functions
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);

	//Enemy functions
	void DebugDraw(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleRayCollision(Edge *edge, double edgeQuantity,
		double rayPortion);
	void FrameIncrement();
	void DrawMinimap(sf::RenderTarget * target);

	//Boss functions
	bool TryComboMove(V2d &comboPos, int comboMoveDuration,
		int moveDurationBeforeStartNextAction,
		V2d &comboOffset);
	void HandleSummonedChildRemoval(Enemy *e);
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