#include "Boss.h"
#include "Session.h"
#include "Actor.h"

Boss::Boss(EnemyType et, ActorParams *ap)
	:Enemy(et, ap)
{
	SetTargetPlayerIndex(0);
	hitboxInfo = new HitboxInfo;
	hitboxManager = NULL;
}

Boss::~Boss()
{
	if (hitboxManager != NULL)
	{
		delete hitboxManager;
	}
}

void Boss::CreateHitboxManager(const std::string &folder)
{
	assert(hitboxManager == NULL);

	hitboxManager = new HitboxManager(folder);
}

void Boss::BossReset()
{
	hitPlayer = false;
	decide = false;
	invincibleFrames = 0;
	stageMgr.Reset();
	enemyMover.Reset();
	prevAction = -1;
	movingToCombo = false;
}

void Boss::StageSetup(int numStages, int hitsPerStage)
{
	stageMgr.Setup(numStages, hitsPerStage);
	maxHealth = HITS_PER_BOSS_DAMAGE * stageMgr.GetTotalHealth();
}

void Boss::Decide()
{
	decide = true;
}

void Boss::SetAction(int a)
{
	prevAction = action;
	action = a;
	frame = 0;
	StartAction();
}

void Boss::ProcessState()
{
	ActionEnded();
	TryCombo();
	CheckEnemyMoverActionsOver();
	TryExecuteDecision();
	HandleAction();
	EndProcessState();
}

void Boss::CheckEnemyMoverActionsOver()
{
	if (IsEnemyMoverAction(action))
	{
		if (enemyMover.IsIdle())
		{
			Decide();
		}
	}
}

void Boss::TryCombo()
{
	bool comboInterrupted = targetPlayer->hitOutOfHitstunLastFrame;
	if (hitPlayer || (movingToCombo && comboInterrupted))
	{
		assert(targetPlayer->hitstunFrames > 2);
		V2d tPos = sess->GetFuturePlayerPos(targetPlayer->hitstunFrames - 2);
		int moveDuration = targetPlayer->hitstunFrames - 4;
		movingToCombo = TryComboMove(tPos, moveDuration);
	}
}

void Boss::EndProcessState()
{
	hitPlayer = false;
	enemyMover.currPosInfo = currPosInfo;
}

void Boss::TryExecuteDecision()
{
	if (decide)
	{
		if (stageMgr.stageChanged)
		{
			SetAction(ChooseActionAfterStageChange());
			stageMgr.stageChanged = false;
		}
		else
		{
			SetAction(ChooseNextAction());
		}
		decide = false;
	}
}

void Boss::Setup()
{
	Enemy::Setup();

	SetupPostFightScenes();
	SetupNodeVectors();
}

void Boss::SetTargetPlayerIndex(int ind)
{
	targetPlayerIndex = ind;
	targetPlayer = sess->GetPlayer(targetPlayerIndex);
}

void Boss::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames + 1;
}

void Boss::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
	else
	{
		Enemy::UpdateEnemyPhysics();
	}
}

bool Boss::CanBeHitByPlayer()
{
	return invincibleFrames == 0;
}

void Boss::FrameIncrement()
{
	enemyMover.FrameIncrement();
	currPosInfo = enemyMover.currPosInfo;

	if (invincibleFrames > 0)
	{
		--invincibleFrames;
	}
}

int Boss::ChooseNextAction()
{
	int d;
	do
	{
		d = stageMgr.AlwaysGetNextOption();
	} while (!IsDecisionValid(d));

	return d;
}

void Boss::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 1)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			//currently cant be reached. adjust when bosses are needed in boss rush/levels later
			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else if (numHealth == 1)
		{
			ActivatePostFightScene();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();

			if (numHealth % HITS_PER_BOSS_DAMAGE == 0)
			{
				stageMgr.TakeHit();
				invincibleFrames = 60;
			}
		}

		receivedHit = NULL;
	}
}