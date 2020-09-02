#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Bird.h"
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


Bird::Bird(ActorParams *ap)
	:Enemy(EnemyType::EN_BIRDBOSS, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(PUNCH, 0, 0);

	actionLength[PUNCH] = 14;
	animFactor[PUNCH] = 3;
	reachPointOnFrame[PUNCH] = 0;

	actionLength[KICK] = 10;
	animFactor[KICK] = 3;
	reachPointOnFrame[KICK] = 0;

	actionLength[MOVE] = 2;
	animFactor[MOVE] = 1;
	reachPointOnFrame[MOVE] = 0;

	ts_move = sess->GetSizedTileset("Bosses/Bird/intro_256x256.png");

	ts_punch = sess->GetSizedTileset("Bosses/Bird/punch_256x256.png");

	ts_kick = sess->GetSizedTileset("Bosses/Bird/kick_256x256.png");

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

	move = ms.AddLineMovement(V2d(), V2d(), CubicBezier(), 0);

	predictCircle.setFillColor(Color::Red);
	predictCircle.setRadius(20);
	predictCircle.setOrigin(predictCircle.getLocalBounds().width / 2,
		predictCircle.getLocalBounds().height / 2);

	ResetEnemy();
}

void Bird::LoadParams()
{
	ifstream is;
	is.open("Resources/Bosses/Bird/birdparams.json");

	assert(is.is_open());

	json j;
	is >> j;

	HitboxInfo::SetupHitboxLevelInfo(j["punch"], hitboxInfos[PUNCH]);
	HitboxInfo::SetupHitboxLevelInfo(j["kick"], hitboxInfos[KICK]);
}

void Bird::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	if (facingRight)
	{
		hitBody.hitboxInfo->kbDir.x = hitboxInfos[action].kbDir.x;	
	}
	else
	{
		hitBody.hitboxInfo->kbDir.x = -hitboxInfos[action].kbDir.x;
	}
}

void Bird::ResetEnemy()
{
	fireCounter = 0;
	facingRight = true;

	action = PUNCH;
	frame = 0;
	
	SetHitboxInfo(PUNCH);

	predict = false;
	hitPlayer = false;
	moveFrames = 0;

	ms.currMovement = NULL;

	actionQueueIndex = 0;

	DefaultHitboxesOn();

	UpdateSprite();
}

void Bird::SetHitboxInfo(int a)
{
	*hitboxInfo = hitboxInfos[a];
	hitBody.hitboxInfo = hitboxInfo;
}

void Bird::SetCommand(int index, BirdCommand &bc)
{
	actionQueue[index] = bc;
}

void Bird::CalcTargetAfterHit()
{
	sess->ForwardSimulatePlayer(targetPlayerIndex, sess->GetPlayer(targetPlayerIndex)->hitstunFrames);
	targetPos = sess->GetPlayerPos(targetPlayerIndex);
	sess->RevertSimulatedPlayer(targetPlayerIndex);
	predictCircle.setPosition(Vector2f(targetPos));
}

void Bird::DirectKill()
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

void Bird::FrameIncrement()
{
	++fireCounter;

	if (moveFrames > 0)
	{
		--moveFrames;
	}
}

void Bird::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(targetPlayerIndex);
	if (predict || targetPlayer->hitOutOfHitstunLastFrame)
	{
		if (actionQueueIndex == 3)
		{
			dead = true;
			sess->RemoveEnemy(this);
			return;
		}
		CalcTargetAfterHit();
		moveFrames = targetPlayer->hitstunFrames-1;//(hitBody.hitboxInfo->hitstunFrames - 1);
		counterTillAttack = moveFrames - 10;
		move->duration = moveFrames * NUM_MAX_STEPS * 5;
		move->start = GetPosition();
		move->end = targetPos;
		ms.Reset();
		predict = false;
		int nextAction = actionQueue[actionQueueIndex].action + 1;
		moveFrames -= actionLength[nextAction] * animFactor[nextAction] - 10;
		if (moveFrames < 0)
		{
			moveFrames = 0;
		}

		SetHitboxes(NULL, 0);

		action = MOVE;
		frame = 0;
		//++moveFrames;
		
	}
}

void Bird::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case PUNCH:
			frame = 0;
			break;
		case MOVE:
			frame = 0;
			break;
		case KICK:
			frame = 0;
			break;
		}
	}

	if (action == MOVE)
	{
		if (moveFrames == 0)
		{
			action = actionQueue[actionQueueIndex].action + 1;
			facingRight = actionQueue[actionQueueIndex].facingRight;
			SetHitboxInfo(action);
		}



		if (hitPlayer)
		{
			action = MOVE;
			frame = 0;
			predict = true;
			++actionQueueIndex;
		}
	}





	V2d pDir = normalize(sess->GetPlayerPos(1) - GetPosition());
	//if( (fireCounter == 0 || fireCounter == 10 || fireCounter == 20/*framesBetween - 1*/) && slowCounter == 1 )// frame == 0 && slowCounter == 1 )
	if (slowCounter == 1)//&& action == FLY )
	{
		int f = fireCounter % 60;

		if (f % 5 == 0 && f >= 25 && f < 50)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = pDir;
			//launchers[0]->Fire();
		}
	}

	hitPlayer = false;
}

void Bird::IHitPlayer(int index)
{
	hitPlayer = true;
	pauseFrames = hitBody.hitboxInfo->hitlagFrames;
	/*V2d playerPos = sess->GetPlayerPos(index);
	if (playerPos.x > GetPosition().x)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}*/
}

void Bird::UpdateEnemyPhysics()
{
	if (ms.currMovement != NULL)
	{
		if (numPhysSteps == 1)
		{
			ms.Update(slowMultiple, 10);
		}
		else
		{
			ms.Update(slowMultiple);
		}

		currPosInfo.SetPosition(ms.position);

		if (ms.currMovement == NULL)
		{
			DefaultHitboxesOn();
		}
	}
}

void Bird::UpdateSprite()
{
	switch (action)
	{
	case MOVE:
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
	}
	
	sprite.setPosition(GetPositionF());
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

void Bird::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	target->draw(predictCircle);
}

void Bird::HandleHitAndSurvive()
{
	fireCounter = 0;
}

int Bird::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Bird::StoreBytes(unsigned char *bytes)
{
	MyData d;
	memset(&d, 0, sizeof(MyData));
	StoreBasicEnemyData(d);
	d.fireCounter = fireCounter;

	memcpy(bytes, &d, sizeof(MyData));

	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
}

void Bird::SetFromBytes(unsigned char *bytes)
{
	MyData d;
	memcpy(&d, bytes, sizeof(MyData));

	SetBasicEnemyData(d);

	fireCounter = d.fireCounter;

	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
}