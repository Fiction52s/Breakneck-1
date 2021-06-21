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

	shield = new Shield(Shield::ShieldType::T_BLOCK, 16 * scale, 3, this);

	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = 2.0;//3.0;//1.0;

	maxSpeed = 30;//20;

	ts = sess->GetSizedTileset("Enemies/W4/turtle_80x64.png");
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
	delete shield;
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

	UpdateHitboxes();

	UpdateSprite();

	shield->Reset();
	shield->SetPosition(GetPosition());
	currShield = shield;
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

	velocity = rushDir * speed;
}

void Chess::ProcessState()
{
	ActionEnded();


	
	

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < attentionRadius)
		{
			action = CHASE;
			frame = 0;
			//StartRush();
		}
		break;
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
		velocity = V2d(0, 0);
		break;
	case RUSH:
		break;
	case CHASE:
	{
		//V2d futurePos = sess->GetFuturePlayerPos(20);
		//V2d newPos;
		//newPos.x = GetPosition().x;
		//newPos.y = futurePos.y;
		//testCircle.setPosition(Vector2f(newPos));
		
		if (chessType == HORIZ)
		{
			if (dir.x > 0)
			{
				velocity.x += accel;//V2d(0, 10);
				CapVectorLength(velocity, maxSpeed);
			}
			else if (dir.x < 0)
			{
				velocity.x += -accel;//V2d(0, 10);
				CapVectorLength(velocity, maxSpeed);
			}
		}
		else if( chessType == VERT )
		{
			if (dir.y > 0)
			{
				velocity.y += accel;//V2d(0, 10);
				CapVectorLength(velocity, maxSpeed);
			}
			else if (dir.y < 0)
			{
				velocity.y += -accel;//V2d(0, 10);
				CapVectorLength(velocity, maxSpeed);
			}
		}
		else if (chessType == DIAGDOWNRIGHT)
		{
			V2d downRight = normalize(V2d(1, 1));
			double d = dot(dir, downRight);
			if (d > 0)
			{
				velocity += downRight * accel;
				CapVectorLength(velocity, maxSpeed);
			}
			else if (d < 0)
			{
				velocity += downRight * -accel;
				CapVectorLength(velocity, maxSpeed);
			}
		}
		else if (chessType == DIAGUPRIGHT)
		{
			V2d upRight = normalize(V2d(1, -1));
			double d = dot(dir, upRight);
			if (d > 0)
			{
				velocity += upRight * accel;
				CapVectorLength(velocity, maxSpeed);
			}
			else if (d < 0)
			{
				velocity += upRight * -accel;
				CapVectorLength(velocity, maxSpeed);
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

void Chess::UpdateEnemyPhysics()
{
	if (action == RUSH || action == CHASE)
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
		shield->SetPosition(GetPosition());
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