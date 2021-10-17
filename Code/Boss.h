#ifndef __BOSS_H__
#define __BOSS_H__

#include "Enemy.h"
#include "EnemyMover.h"
#include "BossStageManager.h"
#include "RandomPicker.h"
#include "SummonGroup.h"
#include "NodeGroup.h"
#include "HitboxManager.h"
#include "BossCommand.h"

struct BossHealth;
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
	bool actionHitPlayer;
	BossHealth *healthBar;

	int nextAction;

	std::vector<BossCommand> commandQueue;
	int currCommandIndex;

	std::map<int, CollisionBody*> hitBodies;
	std::map<int, HitboxInfo> hitboxInfos;

	std::map<int, V2d> hitOffsetMap;
	std::map<int, int> hitboxStartFrame;

	//std::vector<BossCommand> 

	Boss(EnemyType et, ActorParams *ap);
	virtual ~Boss();
	virtual int GetNumSimulationFramesRequired();
	void CreateHitboxManager(const std::string &folder);
	void QueueCommand(BossCommand &cm);
	void UpdateHitboxes();
	void ProcessHit();
	void SetAction(int a);
	void Init();
	bool TrySetActionToNextAction();
	BossCommand &GetCurrCommand();
	void SetNextComboAction();
	void ResetCommands();
	void SetCombo( std::vector<BossCommand> &commandVec );
	void Setup();
	void Decide();
	void TryCombo();
	void TryExecuteDecision();
	virtual void RespondToTakingFullHit() {}
	virtual void StartAction() {}
	void StageSetup(int numStages,
		int hitsPerStage);
	void EndProcessState();
	virtual void SetupPostFightScenes() {}
	virtual void SetupNodeVectors() {}
	void SetTargetPlayerIndex(int ind);
	virtual void ActionEnded() {}
	virtual void HandleAction() {}
	virtual void ProcessState();
	virtual	bool IsEnemyMoverAction(int a) { return false; }
	virtual bool TryComboMove(V2d &comboPos,
		int comboMoveDuration, int moveDurationBeforeStartNextAction, int remainingDuration,
		V2d &comboOffset ) {return false;}

	virtual void FrameIncrement();
	void SetBasicActiveHitbox();
	void SetupHitboxes(int a, const std::string &name);
	void Wait(int numFrames);

	//virtual void MovementEnded();

	void IHitPlayer(int index = 0);
	void BossReset();
	virtual int ChooseActionAfterStageChange();
	virtual bool IsDecisionValid(int d) { return true; }
	int ChooseNextAction();
	virtual void ActivatePostFightScene() {}
	virtual bool CanBeHitByPlayer();
	virtual void UpdateEnemyPhysics();

	/*PositionInfo CheckGround(PositionInfo &startPos,
		double dist);*/
};

#endif

