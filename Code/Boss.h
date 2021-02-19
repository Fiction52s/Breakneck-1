#ifndef __BOSS_H__
#define __BOSS_H__

#include "Enemy.h"
#include "EnemyMover.h"
#include "BossStageManager.h"
#include "RandomPicker.h"
#include "SummonGroup.h"
#include "NodeGroup.h"
#include "HitboxManager.h"

struct BossCommand
{
	BossCommand()
		:action(0), facingRight(true)
	{}
	int action;
	bool facingRight;
};

struct Boss : Enemy
{
	BossStageManager stageMgr;
	const static int HITS_PER_BOSS_DAMAGE = 3;
	bool hitPlayer;
	int invincibleFrames;
	EnemyMover enemyMover;
	int targetPlayerIndex;
	Actor *targetPlayer;
	bool decide;
	int prevAction;
	HitboxManager *hitboxManager;
	bool movingToCombo;

	std::map<int, V2d> actionTargetOffsets;

	//std::vector<BossCommand> 

	Boss(EnemyType et, ActorParams *ap);
	virtual ~Boss();
	void CreateHitboxManager(const std::string &folder);
	void ProcessHit();
	void SetAction(int a);
	void Setup();
	void Decide();
	void TryCombo();
	void TryExecuteDecision();
	virtual void StartAction() {}
	void StageSetup(int numStages,
		int hitsPerStage);
	void EndProcessState();
	virtual void SetupPostFightScenes() {}
	virtual void SetupNodeVectors() {}
	void SetTargetPlayerIndex(int ind);
	void CheckEnemyMoverActionsOver();
	virtual void ActionEnded() {}
	virtual void HandleAction() {}
	virtual void ProcessState();
	virtual	bool IsEnemyMoverAction(int a) { return false; }
	virtual bool TryComboMove(V2d &comboPos, 
		int comboMoveDuration) {return false;}

	virtual void FrameIncrement();

	//virtual void MovementEnded();

	void IHitPlayer(int index = 0);
	void BossReset();
	virtual int ChooseActionAfterStageChange() = 0;
	virtual bool IsDecisionValid(int d) { return true; }
	int ChooseNextAction();
	virtual void ActivatePostFightScene() {}
	virtual bool CanBeHitByPlayer();
	virtual void UpdateEnemyPhysics();
};

#endif

