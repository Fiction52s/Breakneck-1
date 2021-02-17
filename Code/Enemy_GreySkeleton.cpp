#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_GreySkeleton.h"
#include "Actor.h"
#include "SequenceW7.h"

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


GreySkeleton::GreySkeleton(ActorParams *ap)
	:Enemy(EnemyType::EN_GREYSKELETONBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Gator/dominance_384x384.png");
	sprite.setColor(Color::Green);

	nodeAStr = "A";

	postFightScene = NULL;

	//hitboxInfo = new HitboxInfo;
	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);


	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

GreySkeleton::~GreySkeleton()
{
	if (postFightScene != NULL)
		delete postFightScene;
}

void GreySkeleton::Setup()
{
	Enemy::Setup();

	if (postFightScene == NULL)
	{
		postFightScene = new FinalSkeletonPostFightScene;
		postFightScene->greySkeleton = this;
		postFightScene->Init();
	}
}

void GreySkeleton::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void GreySkeleton::UpdateHitboxes()
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

void GreySkeleton::ResetEnemy()
{
	//orbPool.Reset();
	enemyMover.Reset();

	facingRight = true;

	action = WAIT;
	SetHitboxes(NULL);
	waitFrames = 10;

	//action = PUNCH;
	//SetHitboxInfo(PUNCH);
	//DefaultHitboxesOn();

	hitPlayer = false;
	comboMoveFrames = 0;

	actionQueueIndex = 0;

	frame = 0;

	UpdateSprite();
}

void GreySkeleton::Wait()
{
	action = SEQ_WAIT;
	frame = 0;
	//orbPool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void GreySkeleton::StartFight()
{
	action = WAIT;
	DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	SetHitboxes(NULL);
	waitFrames = 10;
}

void GreySkeleton::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void GreySkeleton::SetCommand(int index, BirdCommand &bc)
{
	actionQueue[index] = bc;
}

void GreySkeleton::DebugDraw(sf::RenderTarget *target)
{
	enemyMover.DebugDraw(target);
}

void GreySkeleton::DirectKill()
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

void GreySkeleton::FrameIncrement()
{
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

void GreySkeleton::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case COMBOMOVE:
			frame = 0;
			break;
		}
	}

	enemyMover.currPosInfo = currPosInfo;

	if (action == MOVE && enemyMover.IsIdle())
	{
		action = WAIT;
		waitFrames = 10;
	}
	else if (action == WAIT && waitFrames == 0)
	{
		int r = rand() % 3;

		auto *nodeVec = sess->GetBossNodeVector(BossFightType::FT_SKELETON2, nodeAStr);
		int vecSize = nodeVec->size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec->at(rNode)->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());



		if (r == 0)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
			//orbPool.Throw(GetPosition(), nodePos, GreySkeletonWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 1)
		{
			enemyMover.SetModeNodeQuadratic(pPos, nodePos, CubicBezier(), 60);
			//orbPool.Throw(GetPosition(), nodePos, GreySkeletonWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 2)
		{
			enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
				10, .5, 60);
			//orbPool.Throw(GetPosition(), nodePos, GreySkeletonWaterOrb::OrbType::NODE_GROW);
		}
		else if (r == 3)
		{

		}


		action = MOVE;
		moveFrames = 60;
	}
	else if (action == COMBOMOVE)
	{
		if (comboMoveFrames == 0)
		{
			action = actionQueue[actionQueueIndex].action + 1;
			facingRight = actionQueue[actionQueueIndex].facingRight;
			SetHitboxInfo(action);
			//only have this on if i dont turn on hitboxes at the end of the movement.
			DefaultHitboxesOn();

		}
	}

	bool comboInterrupted = sess->GetPlayer(targetPlayerIndex)->hitOutOfHitstunLastFrame
		&& comboMoveFrames > 0;
	//added this combo counter thing
	if (hitPlayer || comboInterrupted)
	{
		action = COMBOMOVE;
		frame = 0;
		if (!comboInterrupted)
			++actionQueueIndex;
		SetHitboxes(NULL, 0);

		if (actionQueueIndex == 3)
		{

		}
	}

	hitPlayer = false;
}

void GreySkeleton::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 1)
	{
		numHealth -= 1;

		if (numHealth <= 0)
		{
			if (hasMonitor && !suppressMonitor)
			{
				//sess->CollectKey();
			}

			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			ConfirmKill();
		}
		else
		{
			sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
			ConfirmHitNoKill();
		}

		if (numHealth == 1)
		{
			postFightScene->Reset();
			sess->SetActiveSequence(postFightScene);
		}

		receivedHit = NULL;
	}
}

void GreySkeleton::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void GreySkeleton::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void GreySkeleton::UpdateSprite()
{
	switch (action)
	{
	case WAIT:
	case MOVE:
	case COMBOMOVE:
		sprite.setTexture(*ts_move->texture);
		ts_move->SetSubRect(sprite, 0, !facingRight);
		break;
	}

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void GreySkeleton::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	//orbPool.Draw(target);
}

void GreySkeleton::HandleHitAndSurvive()
{
	//fireCounter = 0;
}

int GreySkeleton::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void GreySkeleton::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	//d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void GreySkeleton::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	//fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}