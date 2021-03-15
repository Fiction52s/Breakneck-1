#include "Boss.h"
#include "Session.h"
#include "Actor.h"

Boss::Boss(EnemyType et, ActorParams *ap)
	:Enemy(et, ap)
{
	SetTargetPlayerIndex(0);
	hitboxInfo = new HitboxInfo;
	hitboxManager = NULL;
	commandQueue.reserve(10);
}

Boss::~Boss()
{
	if (hitboxManager != NULL)
	{
		delete hitboxManager;
	}

	for (auto it = hitBodies.begin(); it != hitBodies.end(); ++it)
	{
		delete (*it).second;
	}
}

int Boss::GetNumSimulationFramesRequired()
{
	return sess->MAX_SIMULATED_FUTURE_PLAYER_FRAMES;
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
	actionHitPlayer = false;
	commandQueue.clear();
	currCommandIndex = 0;
	nextAction = -1;
}

void Boss::QueueCommand(BossCommand &cm)
{
	commandQueue.push_back(cm);
}

void Boss::SetCombo(std::vector<BossCommand> &commandVec)
{
	ResetCommands();
	int size = commandVec.size();
	for (int i = 0; i < size; ++i)
	{
		QueueCommand(commandVec[i]);
	}
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
	actionHitPlayer = false;
	StartAction();
}

BossCommand &Boss::GetCurrCommand()
{
	return commandQueue[currCommandIndex];
}

void Boss::SetNextComboAction()
{
	BossCommand nextComboAction = commandQueue[currCommandIndex];
	SetAction(nextComboAction.action);
	++currCommandIndex;
}

bool Boss::TrySetActionToNextAction()
{
	if (nextAction == -1)
	{
		return false;
	}
	else
	{
		int n = nextAction;
		nextAction = -1;
		SetAction(n);
		return true;
	}
}

void Boss::ProcessState()
{
	if (IsEnemyMoverAction(action))
	{
		if (enemyMover.IsIdle())
		{
			ActionEnded();
		}
	}
	else if(frame == actionLength[action] * animFactor[action])
	{
		ActionEnded();
	}

	TryCombo();
	TryExecuteDecision();
	HandleAction();
	EndProcessState();
}

void Boss::TryCombo()
{
	if (currCommandIndex < commandQueue.size())
	{
		bool comboInterrupted = targetPlayer->hitOutOfHitstunLastFrame;
		if (hitPlayer || (movingToCombo && comboInterrupted))
		{
			assert(targetPlayer->hitstunFrames > 2);
			V2d tPos = sess->GetFuturePlayerPos(targetPlayer->hitstunFrames - 2);
			int moveDuration = targetPlayer->hitstunFrames - 4;

			BossCommand nextComboAction = commandQueue[currCommandIndex];

			int framesRemaining = (actionLength[action] * animFactor[action]) - frame;
			int durationBeforeNextAction =
				moveDuration - (hitboxStartFrame[nextComboAction.action] * animFactor[nextComboAction.action] - 1);// -framesRemaining;

			V2d offset;
			if (hitOffsetMap.find(nextComboAction.action) != hitOffsetMap.end() )
			{
				offset = -hitOffsetMap[nextComboAction.action];//(-100, 0);
				if (!nextComboAction.facingRight)
				{
					offset.x = -offset.x;
				}
			}

			movingToCombo = TryComboMove(tPos, moveDuration,
				durationBeforeNextAction, framesRemaining, offset );
		}
	}
}

void Boss::Wait( int numFrames )
{
	SetAction(0);
	assert(numFrames > 0);
	actionLength[0] = numFrames;
}

void Boss::UpdateHitboxes()
{
	V2d position = GetPosition();

	double ang = GetGroundedAngleRadians();
	//can update this with a universal angle at some point
	if (!hurtBody.Empty())
	{
		hurtBody.SetBasicPos(position, ang);
		hurtBody.GetCollisionBoxes(0).at(0).flipHorizontal = !facingRight;
	}

	if (currHitboxes != NULL)
	{
		std::vector<CollisionBox> *cList = &(currHitboxes->GetCollisionBoxes(currHitboxFrame));
		if (cList != NULL)
			for (auto it = cList->begin(); it != cList->end(); ++it)
			{
				/*if (ground != NULL)
				{
				(*it).globalAngle = angle;
				}
				else
				{
				(*it).globalAngle = 0;
				}*/

				(*it).flipHorizontal = !facingRight;

				V2d pos = GetPosition();

				(*it).globalPosition = pos;
			}

		hitboxInfos[action].flipHorizontalKB = !facingRight;
	}
}

void Boss::SetBasicActiveHitbox()
{
	if (!actionHitPlayer)
	{
		SetHitboxes(hitBodies[action], frame / animFactor[action]);
	}
}

void Boss::SetupHitboxes(int a, const std::string &name)
{
	CollisionBody *hBody = hitboxManager->CreateBody(name, &hitboxInfos[a]);
	hBody->hitboxInfo->hitsThroughInvincibility = true;

	hitBodies[a] = hBody;
	
	hitboxStartFrame[a] = hBody->GetFirstNonEmptyFrame();
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
		else if (nextAction != -1)
		{
			SetAction(nextAction);
		}
		else
		{
			SetAction(ChooseNextAction());
		}
		decide = false;
	}
}

int Boss::ChooseActionAfterStageChange()
{
	return ChooseNextAction();
}

void Boss::Setup()
{
	Enemy::Setup();

	SetupPostFightScenes();
	SetupNodeVectors();
}

void Boss::ResetCommands()
{
	commandQueue.clear();
	currCommandIndex = 0;
}

void Boss::SetTargetPlayerIndex(int ind)
{
	targetPlayerIndex = ind;
	targetPlayer = sess->GetPlayer(targetPlayerIndex);
}

void Boss::IHitPlayer(int index)
{
	hitPlayer = true;
	actionHitPlayer = true;
	UpdateSprite(); //because paused will not show the correct frame
	pauseFrames = currHitboxes->hitboxInfo->hitlagFrames + 1; //+1 so you can wait until the player has chosen DI before predicting
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