#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Chess.h"
#include "Actor.h"
#include "Shield.h"

using namespace std;
using namespace sf;

Chess::Chess(ActorParams *ap)
	:Enemy(EnemyType::EN_CHESS, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 1;
	actionLength[RUSH] = 60;
	actionLength[RECOVER] = 60;
	actionLength[CHASE] = 2;

	/*animFactor[NEUTRAL] = 1;
	animFactor[APPROACH] = 1;
	animFactor[PULSE] = 1;
	animFactor[RECOVER] = 1;*/

	const string &typeName = ap->GetTypeName();
	if (typeName == "chessx")
	{
		chessType = HORIZ;
	}
	else if( typeName == "chessy")
	{
		chessType = VERT;
		sprite.setColor(Color::Black);
	}
	else if (typeName == "chessdiagdownright")
	{
		chessType = DIAGDOWNRIGHT;
		sprite.setColor(Color::Red);
	}
	else if (typeName == "chessdiagupright")
	{
		chessType = DIAGUPRIGHT;
		sprite.setColor(Color::Cyan);
	}
	else
	{
		chessType = HORIZ;
		assert(0);
	}

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 16, 1, GetPosition(), V2d(1, 0), 0, 180, false);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->hType = HitboxInfo::ORANGE;
	launchers[0]->Reset();

	ts_bulletExplode = GetSizedTileset("FX/bullet_explode3_64x64.png");

	//shield = new Shield(Shield::ShieldType::T_BLOCK, 16 * scale, 3, this);
	//RegisterShield(shield);

	accel = 2.0;//3.0;//1.0;

	maxSpeed = 30;//20;

	ts = GetSizedTileset("Enemies/W4/turtle_80x64.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(36);
	cutObject->SetSubRectBack(37);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Chess::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

Chess::~Chess()
{
	currShield = NULL;
	//delete shield;
}

void Chess::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 2.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void Chess::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	data.fireCounter = 0;

	data.velocity = V2d();

	UpdateHitboxes();

	UpdateSprite();

	//shield->Reset();
	//shield->SetPosition(GetPosition());
	//currShield = shield;
}

void Chess::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case RUSH:
			StartRush();
			//action = RECOVER;
			break;
		case RECOVER:
			//StartRush();
			break;
		}
	}
}

void Chess::StartRush()
{
	action = RUSH;
	frame = 0;

	int predictFrame = 30;
	V2d futurePos = sess->GetFuturePlayerPos(predictFrame);

	V2d rushDir = normalize(futurePos - GetPosition());

	double rushDist = length(futurePos - GetPosition());
	double speed = rushDist / predictFrame;
	double time = rushDist / speed;
	actionLength[RUSH] = time + 1;//time * 1.5;

	//testCircle.setPosition(Vector2f(futurePos));

	data.velocity = rushDir * speed;
}

void Chess::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void Chess::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void Chess::DirectKill()
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

	Enemy::DirectKill();
}

void Chess::ProcessState()
{
	ActionEnded();


	
	

	double dist = PlayerDist();
	V2d dir = PlayerDir();



	switch (action)
	{
	case NEUTRAL:
	{
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = CHASE;
			frame = 0;
			//StartRush();
		}
		break;
	}
	case CHASE:
	{
		break;
	}
	case RETURN:
	{
		break;
	}
	case RUSH:
		/*if (dist > ignoreRadius)
		{
			action = NEUTRAL;
			frame = 0;
		}*/
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		data.velocity = V2d(0, 0);
		break;
	case RUSH:
		break;
	case CHASE:
	{
		if (data.fireCounter == 0 && dist < DEFAULT_DETECT_RADIUS)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();
			//launchers[0]->Reset();
			launchers[0]->Fire();
		}
		//V2d futurePos = sess->GetFuturePlayerPos(20);
		//V2d newPos;
		//newPos.x = GetPosition().x;
		//newPos.y = futurePos.y;
		//testCircle.setPosition(Vector2f(newPos));
		
		if (chessType == HORIZ)
		{
			if (dir.x > 0)
			{
				data.velocity.x += accel;//V2d(0, 10);
				CapVectorLength(data.velocity, maxSpeed);
			}
			else if (dir.x < 0)
			{
				data.velocity.x += -accel;//V2d(0, 10);
				CapVectorLength(data.velocity, maxSpeed);
			}
		}
		else if( chessType == VERT )
		{
			if (dir.y > 0)
			{
				data.velocity.y += accel;//V2d(0, 10);
				CapVectorLength(data.velocity, maxSpeed);
			}
			else if (dir.y < 0)
			{
				data.velocity.y += -accel;//V2d(0, 10);
				CapVectorLength(data.velocity, maxSpeed);
			}
		}
		else if (chessType == DIAGDOWNRIGHT)
		{
			V2d downRight = normalize(V2d(1, 1));
			double d = dot(dir, downRight);
			if (d > 0)
			{
				data.velocity += downRight * accel;
				CapVectorLength(data.velocity, maxSpeed);
			}
			else if (d < 0)
			{
				data.velocity += downRight * -accel;
				CapVectorLength(data.velocity, maxSpeed);
			}
		}
		else if (chessType == DIAGUPRIGHT)
		{
			V2d upRight = normalize(V2d(1, -1));
			double d = dot(dir, upRight);
			if (d > 0)
			{
				data.velocity += upRight * accel;
				CapVectorLength(data.velocity, maxSpeed);
			}
			else if (d < 0)
			{
				data.velocity += upRight * -accel;
				CapVectorLength(data.velocity, maxSpeed);
			}
		}
		
		//currPosInfo.position = newPos;
		//velocity = V2d();
		//V2d futureDir = normalize(futurePos - GetPosition());
		//velocity = 20.0 * futureDir;//+= futureDir * accel;
		//CapVectorLength(velocity, maxSpeed);
		break;
	}
		
	}
}

void Chess::FrameIncrement()
{
	if (action == CHASE && PlayerDist() < DEFAULT_DETECT_RADIUS)
	{
		++data.fireCounter;
		if (data.fireCounter == 60)
		{
			data.fireCounter = 0;
		}
	}
}

void Chess::UpdateEnemyPhysics()
{
	if (action == RUSH || action == CHASE)
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		//shield->SetPosition(GetPosition());
	}
}

void Chess::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		//sprite.setColor(Color::White);
		break;
	case RUSH:
		//sprite.setColor(Color::Green);
		break;
	case RECOVER:
		//sprite.setColor(Color::Black);
		break;
	}

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


	//testCircle.setPosition(GetPositionF());
}

void Chess::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

int Chess::GetNumStoredBytes()
{
	return sizeof(MyData);// +shield->GetNumStoredBytes();
}

void Chess::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	//shield->StoreBytes(bytes);
	//bytes += shield->GetNumStoredBytes();
}

void Chess::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	//shield->SetFromBytes(bytes);
	//bytes += shield->GetNumStoredBytes();
}