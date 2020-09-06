#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Coyote.h"
#include "Actor.h"

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
	:Enemy(EnemyType::EN_COYOTEBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	targetPlayerIndex = 0;

	actionLength[COMBOMOVE] = 2;
	animFactor[COMBOMOVE] = 1;
	reachPointOnFrame[COMBOMOVE] = 0;

	//ts_move = sess->GetSizedTileset("Bosses/Bird/intro_256x256.png");

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

	//BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);


	ts_bulletExplode = sess->GetTileset("FX/bullet_explode3_64x64.png", 64, 64);

	myCircle.setFillColor(Color::Magenta);
	myCircle.setRadius(20);
	myCircle.setOrigin(myCircle.getLocalBounds().width / 2,
		myCircle.getLocalBounds().height / 2);

	ResetEnemy();
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
	playerComboer.Reset();
	stopStartPool.Reset();
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

void Coyote::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

//void Coyote::SetCommand(int index, BirdCommand &bc)
//{
//	actionQueue[index] = bc;
//}

void Coyote::DebugDraw(sf::RenderTarget *target)
{
	playerComboer.DebugDraw(target);
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

void Coyote::UpdatePreFrameCalculations()
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

void Coyote::ProcessState()
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

		auto &nodeVec = sess->GetBossNodeVector(BossFightType::FT_BIRD, nodeAStr);
		int vecSize = nodeVec.size();
		int rNode = rand() % vecSize;

		V2d nodePos = nodeVec[rNode]->pos;

		V2d pPos = sess->GetPlayerPos(0);
		V2d pDir = normalize(pPos - GetPosition());


		if (r == 0)
		{
			enemyMover.SetModeNodeLinearConstantSpeed(nodePos, CubicBezier(), 10);
			stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 1)
		{
			enemyMover.SetModeNodeQuadratic(pPos, nodePos, CubicBezier(), 60);
			stopStartPool.Throw(GetPosition(), pDir);
		}
		else if (r == 2)
		{
			enemyMover.SetModeChase(&sess->GetPlayer(0)->position, V2d(0, 0),
				10, .5, 60);
			stopStartPool.Throw(GetPosition(), pDir);
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

	myCircle.setPosition(GetPositionF());

	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Coyote::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(myCircle);
	//DrawSprite(target, sprite);
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