#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Coyote.h"
#include "Actor.h"
#include "SequenceW3.h"
#include "Enemy_Firefly.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )


Coyote::Coyote(ActorParams *ap)
	:Enemy(EnemyType::EN_COYOTEBOSS, ap),
	fireflySummonGroup( this, 
		new BasicAirEnemyParams(sess->types["firefly"], 1),
		5, 2, 1 )
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	stageMgr.AddBossStage(4);
	stageMgr.AddBossStage(4);
	stageMgr.AddBossStage(4);
	stageMgr.AddBossStage(4);

	decidePickers = new RandomPicker[stageMgr.numStages];

	maxHealth = 3 * stageMgr.GetTotalHealth();

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	actionLength[SUMMON] = 60;

	ts_move = sess->GetSizedTileset("Bosses/Coyote/coy_stand_80x64.png");

	nodeAVec = NULL;

	nodeAStr = "A";

	//hitboxInfo = new HitboxInfo;

	postFightScene = NULL;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 50;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);

	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

Coyote::~Coyote()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}

	delete[] decidePickers;
}

void Coyote::Setup()
{
	Enemy::Setup();
	if (postFightScene != NULL)
	{
		postFightScene = new CoyotePostFightScene;
		postFightScene->coy = this;
		postFightScene->Init();
	}
	
	nodeAVec = sess->GetBossNodeVector(BossFightType::FT_COYOTE, "A");
	assert(nodeAVec != NULL);

	int numNodes = nodeAVec->size();
	nodePicker.ReserveNumOptions(numNodes);
	nodePicker.Reset();

	for (int i = 0; i < numNodes; ++i)
	{
		nodePicker.AddActiveOption(i);
	}
}

void Coyote::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Coyote::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	if (hitBody.hitboxInfo != NULL)
	{
		if (facingRight)
		{
			hitBody.hitboxInfo->kbDir.x = hitboxInfos[action].kbDir.x;
		}
		else
		{
			hitBody.hitboxInfo->kbDir.x = -hitboxInfos[action].kbDir.x;
		}
	}
}

void Coyote::ResetEnemy()
{
	stopStartPool.Reset();
	enemyMover.Reset();
	stageMgr.Reset();
	fireflySummonGroup.Reset();

	fireCounter = 0;
	facingRight = true;

	invincibleFrames = 0;

	action = WAIT;
	SetHitboxes(NULL);
	waitFrames = 10;

	StartFight();

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	hitPlayer = false;
	comboMoveFrames = 0;

	frame = 0;

	UpdateSprite();
}

void Coyote::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 1)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else if (numHealth == 1)
		{
			postFightScene->Reset();
			sess->SetActiveSequence(postFightScene);
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();

			if (numHealth % 3 == 0)
			{
				if (!stageMgr.TakeHit())
				{
					NextStage();
				}
				invincibleFrames = 60;
			}
		}

		

		receivedHit = NULL;
	}
}

void Coyote::NextStage()
{
	stageChanged = true;
	switch (stageMgr.GetCurrStage())
	{
	case 1:
		break;
	case 2:

		break;
	}
}

void Coyote::Wait()
{
	action = SEQ_WAIT;
	frame = 0;
	stopStartPool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	facingRight = false;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Coyote::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void Coyote::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void Coyote::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[0]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit = NULL;
}

void Coyote::FrameIncrement()
{
	if (action == SEQ_WAIT)
		return;

	++fireCounter;

	if (comboMoveFrames > 0)
	{
		--comboMoveFrames;
	}

	if (moveFrames > 0)
	{
		--moveFrames;
	}

	if (waitFrames > 0)
	{
		--waitFrames;
	}

	enemyMover.FrameIncrement();
	currPosInfo = enemyMover.currPosInfo;
}

void Coyote::StartFight()
{
	action = WAIT;
	DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
}

void Coyote::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case SEQ_WAIT:
			frame = 0;
			break;
		case COMBOMOVE:
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if (action == MOVE && enemyMover.IsIdle())
	{
		action = WAIT;
		waitFrames = 1;//10
	}
	else if (action == WAIT && waitFrames == 0)
	{
		int r = rand() % 4;

		auto *nodeVec = sess->GetBossNodeVector(BossFightType::FT_COYOTE, nodeAStr);
		int vecSize = nodeVec->size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec->at(rNode)->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());

		enemyMover.currPosInfo.SetAerial();
		currPosInfo.SetAerial();

		//r = 3; //to shoot nothing
		if (r == 0)
		{
			action = MOVE;
			moveFrames = 60;
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
			stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 1)
		{
			action = MOVE;
			moveFrames = 60;
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
			//enemyMover.SetModeNodeQuadratic(pPos, nodePos, CubicBezier(), 60);
			stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 2)
		{
			action = MOVE;
			moveFrames = 60;
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			//enemyMover.SetModeNodeLinear(nodePos, CubicBezier(), 60);
			//enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
			//	10, .5, 60);
			stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 3)
		{
			action = SUMMON;
			frame = 0;
		}


		
	}
	else if (action == COMBOMOVE)
	{
		if (comboMoveFrames == 0)
		{
			//action = actionQueue[actionQueueIndex].action + 1;
			//facingRight = actionQueue[actionQueueIndex].facingRight;
			SetHitboxInfo(action);
			//only have this on if i dont turn on hitboxes at the end of the movement.
			DefaultHitboxesOn();

		}
	}
	else if (action == SUMMON && frame == 20 && slowCounter == 1)
	{
		fireflySummonGroup.Summon();
	}

	bool comboInterrupted = sess->GetPlayer(targetPlayerIndex)->hitOutOfHitstunLastFrame
		&& comboMoveFrames > 0;
	//added this combo counter thing
	if (hitPlayer || comboInterrupted)
	{
		action = COMBOMOVE;
		frame = 0;
		//if (!comboInterrupted)
		//	++actionQueueIndex;
		SetHitboxes(NULL, 0);

		//if (actionQueueIndex == 3)
		{

		}
	}

	hitPlayer = false;
}

bool Coyote::IsDecisionValid(int d)
{
	if (d == SUMMON && !fireflySummonGroup.CanSummon())
	{
		return false;
	}

	return true;
}

void Coyote::ChooseNextAction()
{
	int currStage = stageMgr.GetCurrStage();
	int d;

	do
	{
		d = decidePickers[currStage].AlwaysGetNextOption();
	} while (!IsDecisionValid(d));


	action = d;
	frame = 0;
}

void Coyote::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void Coyote::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void Coyote::UpdateSprite()
{
	/*switch (action)
	{
	case WAIT:
	case MOVE:
	case COMBOMOVE:
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 2, !facingRight);
		break;
	case PUNCH:
		sprite.setTexture(*ts_punch->texture);
		ts_punch->SetSubRect(sprite, frame / animFactor[action] + 14, !facingRight);
		break;
	case KICK:
		sprite.setTexture(*ts_kick->texture);
		ts_kick->SetSubRect(sprite, frame / animFactor[action] + 6, !facingRight);
		break;
	}*/

	sprite.setTexture(*ts_move->texture);
	ts_move->SetSubRect(sprite, 0, !facingRight);

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Coyote::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	stopStartPool.Draw(target);
}

void Coyote::HandleHitAndSurvive()
{
	fireCounter = 0;
}

int Coyote::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Coyote::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void Coyote::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}

void Coyote::InitEnemyForSummon(SummonGroup *group,
	Enemy *e)
{
	if (group == &fireflySummonGroup)
	{
		PoiInfo *summonNode;

		summonNode = nodeAVec->at(nodePicker.AlwaysGetNextOption());

		e->startPosInfo.SetAerial(summonNode->pos);
	}
}