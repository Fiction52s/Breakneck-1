#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Tiger.h"
#include "Actor.h"
#include "SequenceW4.h"

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


Tiger::Tiger(ActorParams *ap)
	:Enemy(EnemyType::EN_TIGERBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Coyote/coy_stand_80x64.png");
	sprite.setColor(Color::Red);

	postFightScene = NULL;

	level = ap->GetLevel();

	nodeAStr = "A";

	hitboxInfo = new HitboxInfo;
	/*hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 0;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 50;
	hitboxInfo->kbDir = normalize(V2d(1, -2));
	hitboxInfo->gravMultiplier = .5;
	hitboxInfo->invincibleFrames = 15;*/

	LoadParams();

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);


	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	ResetEnemy();
}

Tiger::~Tiger()
{
	if (postFightScene != NULL)
	{
		delete postFightScene;
	}
}

void Tiger::LoadParams()
{
	/*ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);*/
}

void Tiger::UpdateHitboxes()
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

void Tiger::ResetEnemy()
{
	currPosInfo = startPosInfo;

	playerComboer.Reset();
	snakePool.Reset();
	enemyMover.Reset();

	fireCounter = 0;
	facingRight = true;

	action = WAIT;
	SetHitboxes(NULL);
	waitFrames = 10;

	//action = PUNCH;
	//SetHitboxInfo(PUNCH);
	//DefaultHitboxesOn();

	hitPlayer = false;
	comboMoveFrames = 0;

	//actionQueueIndex = 0;




	frame = 0;

	UpdateSprite();
}

void Tiger::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

//void Tiger::SetCommand(int index, BirdCommand &bc)
//{
//	actionQueue[index] = bc;
//}

void Tiger::DebugDraw(sf::RenderTarget *target)
{
	playerComboer.DebugDraw(target);
	enemyMover.DebugDraw(target);
}

void Tiger::DirectKill()
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

void Tiger::FrameIncrement()
{
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

void Tiger::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(targetPlayerIndex);

	if (playerComboer.CanPredict(targetPlayerIndex))
	{
		/*if (actionQueueIndex == 3)
		{
		dead = true;
		sess->RemoveEnemy(this);
		return;
		}*/

		playerComboer.UpdatePreFrameCalculations(targetPlayerIndex);
		targetPos = playerComboer.GetTargetPos();

		comboMoveFrames = targetPlayer->hitstunFrames - 1;//(hitBody.hitboxInfo->hitstunFrames - 1);
		counterTillAttack = comboMoveFrames - 10;

		//enemyMover.SetModeNodeJump(targetPos, 200);
		enemyMover.SetModeNodeProjectile(targetPos, V2d(0, 1.0), 200);
		//enemyMover.SetModeNodeLinear(targetPos, CubicBezier(), comboMoveFrames);

		//int nextAction = //actionQueue[actionQueueIndex].action + 1;
		//comboMoveFrames -= actionLength[nextAction] * animFactor[nextAction] - 10;

		if (comboMoveFrames < 0)
		{
			comboMoveFrames = 0;
		}

		SetHitboxes(NULL, 0);

		action = COMBOMOVE;
		frame = 0;
		hitPlayer = false;
	}
}

void Tiger::ProcessState()
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
		waitFrames = 1;//10
		//currPosInfo.SetGround(
		//	targetNode->poly, targetNode->edgeIndex, targetNode->edgeQuantity);
		//enemyMover.currPosInfo = currPosInfo;
	}
	else if (action == WAIT && waitFrames == 0)
	{
		
		int r = rand() % 3;

		auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_TIGER, nodeAStr);
		int vecSize = nodeVec.size();
		int rNode = rand() % vecSize;

		targetNode = nodeVec[rNode];

		V2d nodePos = targetNode->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());

		if (r == 0)
		{
			int gr = rand() % 2;

			double grindSpeed = 20;
			if (gr == 0)
			{
				enemyMover.SetModeGrind(grindSpeed, 120);
			}
			else if (gr == 1)
			{
				enemyMover.SetModeGrind(-grindSpeed, 120);
			}

			//snakePool.Throw(GetPosition(), pDir);
		}
		else if (r == 1)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();

			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			enemyMover.SetDestNode(nodeVec[rNode]);

			//snakePool.Throw(GetPosition(), pDir);
		}
		else if (r == 2)
		{
			enemyMover.currPosInfo.SetAerial();
			currPosInfo.SetAerial();
			enemyMover.SetModeNodeProjectile(nodePos, V2d(0, 1.5), 200);
			//enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 30);
			enemyMover.SetDestNode(nodeVec[rNode]);
			//snakePool.Throw(GetPosition(), pDir);
			
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
			//action = actionQueue[actionQueueIndex].action + 1;
			//facingRight = actionQueue[actionQueueIndex].facingRight;
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
		playerComboer.PredictNextFrame();
		//if (!comboInterrupted)
		//	++actionQueueIndex;
		SetHitboxes(NULL, 0);

		//if (actionQueueIndex == 3)
		{

		}
	}

	hitPlayer = false;
}

void Tiger::ProcessHit()
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
			if (level == 1)
			{
				postFightScene->Reset();
				sess->SetActiveSequence(postFightScene);
			}
			/*else if (level == 2)
			{
				postFightScene2->Reset();
				sess->SetActiveSequence(postFightScene2);
			}*/
		}

		receivedHit = NULL;
	}
}

void Tiger::Setup()
{
	Enemy::Setup();

	postFightScene = new TigerPostFightScene;
	postFightScene->tiger = this;
	postFightScene->Init();
}

void Tiger::Wait()
{
	action = SEQ_WAIT;
	frame = 0;
	snakePool.Reset();
	SetCurrPosInfo(startPosInfo);
	enemyMover.currPosInfo = currPosInfo;
	enemyMover.Reset();
	HurtboxesOff();
	HitboxesOff();
}

void Tiger::StartFight()
{
	action = WAIT;
	//DefaultHitboxesOn();
	DefaultHurtboxesOn();
	frame = 0;
	SetHitboxes(NULL);
	waitFrames = 10;
}

void Tiger::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
}

void Tiger::UpdateEnemyPhysics()
{
	if (!enemyMover.IsIdle())
	{
		enemyMover.UpdatePhysics(numPhysSteps, slowMultiple);
		currPosInfo = enemyMover.currPosInfo;
	}
}

void Tiger::UpdateSprite()
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

void Tiger::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
	snakePool.Draw(target);
}

void Tiger::HandleHitAndSurvive()
{
	fireCounter = 0;
}

int Tiger::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Tiger::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void Tiger::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}