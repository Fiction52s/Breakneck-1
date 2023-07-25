#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gorilla.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )

void Gorilla::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
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

Gorilla::Gorilla( ActorParams *ap )
	:Enemy( EnemyType::EN_GORILLA, ap )
{
	//RegisterCollisionBody(data.wallHitBody);

	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);
	SetLevel(ap->GetLevel());

	//SetSlowable(false);

	//followFrames = 60;

	//idealRadius = 500;//300;
	wallAmountCloser = 50;//200;

	//wallWidth = 300;//600;//400;
	
	/*IDLE,
		FOLLOW,
		ATTACK,
		RECOVER,*/

	actionLength[IDLE] = 2;
	actionLength[FOLLOW] = 30;
	actionLength[ATTACK] = 7;
	actionLength[RECOVER] = 2;


	animFactor[IDLE] = 1;
	animFactor[FOLLOW] = 1;
	animFactor[ATTACK] = 4;
	animFactor[RECOVER] = 15;//5;

	action = IDLE;

	ts = GetSizedTileset( "Enemies/W6/gorilla_320x256.png");

	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 5;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHurtBodySetup(75,0, V2d( -16, 20 ), V2d());
	BasicCircleHitBodySetup(60, 0, V2d( -10, 0 ), V2d());

	hitBody.hitboxInfo = hitboxInfo;

	createWallFrame = 6 * animFactor[ATTACK];//20;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectBack(11);
	cutObject->SetSubRectFront(10);

	ResetEnemy();
}

Gorilla::~Gorilla()
{
}

void Gorilla::ResetEnemy()
{
	action = IDLE;

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	wallPool.Reset();

	data.velocity = V2d(0, 0);
	
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	dead = false;
	frame = 0;
	
	receivedHit.SetEmpty();

	UpdateHitboxes();
	UpdateSprite();
}

void Gorilla::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case FOLLOW:
			data.velocity = V2d(0, 0);
			action = ATTACK;
			frame = 0;
			break;
		case ATTACK:
			action = RECOVER;
			frame = 0;
			break;
		case RECOVER:
			{
			action = FOLLOW;
			frame = 0;

			if( playerPos.x < GetPosition().x )
			{
				facingRight = false;
			}
			else
			{
				facingRight = true;
			}

			break;
			}
			
		}
	}
}

void Gorilla::ProcessState()
{
	ActionEnded();

	V2d playerPos = sess->GetPlayer( 0 )->position;
	V2d myPos = GetPosition();
	double dist = PlayerDist();

	switch( action )
	{
	case IDLE:
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = ATTACK;
			frame = 0;
		}
		break;
	case FOLLOW:
		{
		if (playerPos.x < GetPosition().x)
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}
			//cout << "follow" << endl;	
		}
		break;
	case ATTACK:

		//cout << "attack" << endl;
		if( frame == createWallFrame && slowCounter == 1)
		{
			V2d pDir = PlayerDir();
			wallPool.Throw(myPos + pDir * wallAmountCloser, pDir);

			//V2d test = myPos - playerPos;
	
			//V2d playerDir = PlayerDir();//-normalize(data.origOffset );

			//CollisionBox &wallHitbox = data.wallHitBody.GetCollisionBoxes(0).front();

			//wallHitbox.globalPosition = myPos + playerDir * wallAmountCloser;
			//wallHitbox.globalAngle = atan2( playerDir.x, -playerDir.y );

			//wallSprite.setPosition( wallHitbox.globalPosition.x, 
			//	wallHitbox.globalPosition.y );
			//wallSprite.setRotation( wallHitbox.globalAngle / PI * 180.0 );

			//currWallHitboxes = &data.wallHitBody;

			//data.wallVel = playerDir * 10.0;//2.0;
		}
		break;
	case RECOVER:
		//cout << "recover" << endl;
		break;
	}

	switch (action)
	{
	case FOLLOW:
	{
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			data.velocity = PlayerDir() * 4.0;
		}
		else if (dist > DEFAULT_IGNORE_RADIUS)
		{
			data.velocity = V2d(0, 0);
		}
		break;
	}
	case RECOVER:
	{
		if (playerPos.x < GetPosition().x)
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}

		//cout << "follow" << endl;	
		break;
	}
	}
}

void Gorilla::UpdateEnemyPhysics()
{
	V2d movementVec = data.velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;	
}


void Gorilla::UpdateSprite()
{
	V2d diff = sess->GetPlayerPos(0) - GetPosition();
	double lenDiff = length(diff);
	IntRect ir;
	switch (action)
	{
	case IDLE:
		ir = ts->GetSubRect(1);
		break;
	case FOLLOW:
		ir = ts->GetSubRect(1);
		break;
	case ATTACK:
		ir = ts->GetSubRect(frame / animFactor[ATTACK] + 1);
		break;
	case RECOVER:
		ir = ts->GetSubRect(frame / animFactor[RECOVER] + 8);
		break;
	}

	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}

	sprite.setTextureRect(ir);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Gorilla::EnemyDraw( sf::RenderTarget *target )
{
	DrawSprite(target, sprite);

	wallPool.Draw(target);
}

void Gorilla::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);

	wallPool.DebugDraw(target);
}

void Gorilla::AddToGame()
{
	Enemy::AddToGame();

	wallPool.SetEnemyIDsAndAddToGame();
}

int Gorilla::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Gorilla::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Gorilla::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}