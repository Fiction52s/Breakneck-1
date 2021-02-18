#ifndef __ENEMY_BIRD_H__
#define __ENEMY_BIRD_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "SuperCommands.h"
#include "EnemyMover.h"
#include "Enemy_BirdShuriken.h"
#include "RandomPicker.h"
#include "BossStageManager.h"
#include "SummonGroup.h"
#include "NodeGroup.h"

struct BirdPostFightScene;
struct BirdPostFight2Scene;
struct BirdPostFight3Scene;

struct Bat;



struct Bird : Enemy, Summoner
{
	enum Action
	{
		DECIDE,
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

	int moveFrames;
	void NextStage();
	bool stageChanged;

	BossStageManager stageMgr;

	SummonGroup batSummonGroup;

	NodeGroup nodeGroupA;
	NodeGroup nodeGroupB;

	BirdPostFightScene * postFightScene;
	BirdPostFight2Scene *postFightScene2;
	BirdPostFight3Scene *postFightScene3;

	int invincibleFrames;

	BirdShurikenPool shurPool;
	EnemyMover enemyMover;

	int fireCounter;

	Tileset *ts_bulletExplode;
	int comboMoveFrames;

	std::map<int, int> hitboxStartFrame;

	bool hitPlayer;

	BirdCommand actionQueue[3];
	int actionQueueIndex;

	int targetPlayerIndex;

	HitboxInfo hitboxInfos[A_Count];

	Tileset *ts_punch;
	Tileset *ts_kick;
	Tileset *ts_move;

	int counterTillAttack;

	V2d targetPos;
	int framesToArrive;
	int oldAction;

	Bird(ActorParams *ap);
	~Bird();

	void Wait(int numFrames);

	void Setup();
	bool IsDecisionValid(int d);
	void MoveToCombo();
	void ChooseNextAction();
	bool IsMovementAction(int a);
	int SetLaunchersStartIndex(int ind);
	void SequenceWait();
	void Decide();
	void ProcessHit();
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
	bool CanBeHitByPlayer();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);
	void InitEnemyForSummon(SummonGroup *group,
		Enemy *e);


};

#endif