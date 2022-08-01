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
	:Enemy( EnemyType::EN_GORILLA, ap ),approachAccelBez( 1,.01,.86,.32 ) 
{
	SetNumActions(A_Count);
	SetEditorActions(ATTACK, ATTACK, 0);
	SetLevel(ap->GetLevel());

	SetSlowable(false);

	followFrames = 60;

	idealRadius = 500;//300;
	wallAmountCloser = 200;

	wallWidth = 600;//400;
	
	actionLength[NEUTRAL] = 2;
	actionLength[WAKEUP] = 20;
	actionLength[ALIGN] = 60;
	actionLength[FOLLOW] = followFrames;
	actionLength[ATTACK] = 4;//60;
	actionLength[RECOVER] = 2;//60;


	animFactor[NEUTRAL] = 1;
	animFactor[WAKEUP] = 1;
	animFactor[ALIGN] = 1;
	animFactor[FOLLOW] = 1;
	animFactor[ATTACK] = 15;
	animFactor[RECOVER] = 30;

	action = WAKEUP;

	

	approachFrames = 180 * 3;

	animationFactor = 5;

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

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);

	hitBody.hitboxInfo = hitboxInfo;

	wallHitboxWidth = wallWidth;
	wallHitboxHeight = 50;

	wallHitboxInfo = new HitboxInfo;
	wallHitboxInfo->damage = 180;
	wallHitboxInfo->drainX = 0;
	wallHitboxInfo->drainY = 0;
	wallHitboxInfo->hitlagFrames = 5;
	wallHitboxInfo->hitstunFrames = 10;
	wallHitboxInfo->knockback = 4;
	wallHitboxInfo->hType = HitboxInfo::MAGENTA;

	data.wallHitBody.SetupNumFrames(1);
	data.wallHitBody.SetupNumBoxesOnFrame(0, 1);
	data.wallHitBody.AddBasicRect(0, wallHitboxWidth/2, wallHitboxHeight/2, 0, V2d());
	
	data.wallHitBody.hitboxInfo = wallHitboxInfo;
	

	ts_wall = GetSizedTileset("Enemies/W6/gorillawall_400x50.png");
	wallSprite.setTexture( *ts_wall->texture );
	wallSprite.setTextureRect( ts_wall->GetSubRect( 0 ) );
	wallSprite.setScale(scale * wallWidth / 400.0, scale);
	wallSprite.setOrigin( wallSprite.getLocalBounds().width / 2, wallSprite.getLocalBounds().height / 2 );

	facingRight = true;

	origFacingRight = facingRight;

	createWallFrame = 20;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectBack(9);
	cutObject->SetSubRectFront(8);

	ResetEnemy();
}

Gorilla::~Gorilla()
{
	delete wallHitboxInfo;
}

void Gorilla::ResetEnemy()
{
	action = WAKEUP;
	facingRight = origFacingRight;

	data.latchStartAngle = 0;
	data.latchedOn = false;
	data.totalFrame = 0;
	data.currWallHitboxes = NULL;
	
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	dead = false;
	frame = 0;
	data.basePos = startPosInfo.GetPosition();
	
	receivedHit = NULL;

	UpdateHitboxes();
	UpdateSprite();
	
}

//void Gorilla::UpdateHitboxes()
//{
//	Enemy::UpdateHitboxes();
//	wallHitBody.SetBasicPos(pos, 0);
//	
//}

void Gorilla::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case WAKEUP:
		{
			action = ALIGN;
			data.alignFrames = 0;
			frame = 0;
			data.physStepIndex = 0;
			if (playerPos.x < GetPosition().x)
			{
				facingRight = false;
			}
			else
			{
				facingRight = true;
			}

			data.latchedOn = true;
			data.offsetPlayer = data.basePos - playerPos;
			data.origOffset = data.offsetPlayer;
			V2d offsetDir = normalize(data.offsetPlayer);

			data.basePos = playerPos;

			double currRadius = length(data.offsetPlayer);
			data.alignMoveFrames = 60;// *5 * NUM_STEPS;
			break;
		}
		case ALIGN:
			frame = 0;
			break;
		case FOLLOW:
			action = ATTACK;
			frame = 0;
			break;
		case ATTACK:
			action = RECOVER;
			frame = 0;
			break;
		case RECOVER:
			{
				data.latchedOn = true;
				data.offsetPlayer = data.basePos - playerPos;
				data.origOffset = data.offsetPlayer;
				V2d offsetDir = normalize(data.offsetPlayer );

				data.basePos = playerPos;
				data.currWallHitboxes = NULL;

				action = ALIGN;
				double currRadius = length(data.offsetPlayer );
				data.alignMoveFrames = 60;
				frame = 0;
				data.physStepIndex = 0;
				data.alignFrames = 0;

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

	switch( action )
	{
	case NEUTRAL:
		if (PlayerDist() < 800)
		{
			action = WAKEUP;
			frame = 0;
		}
		break;
	case WAKEUP:
		{
		}
		break;
	case ALIGN:
		{
			if(data.alignFrames >= data.alignMoveFrames )
			{
				action = FOLLOW;
				frame = 0;
			}
			else
			{
				++data.alignFrames;
			}
		}
		break;
	case FOLLOW:
		{
			//cout << "follow" << endl;	
		}
		break;
	case ATTACK:

		//cout << "attack" << endl;
		if( frame == createWallFrame )
		{
			V2d test = myPos - playerPos;
	
			V2d playerDir = -normalize(data.origOffset );

			CollisionBox &wallHitbox = data.wallHitBody.GetCollisionBoxes(0).front();

			wallHitbox.globalPosition = myPos + playerDir * wallAmountCloser;
			wallHitbox.globalAngle = atan2( playerDir.x, -playerDir.y );

			wallSprite.setPosition( wallHitbox.globalPosition.x, 
				wallHitbox.globalPosition.y );
			wallSprite.setRotation( wallHitbox.globalAngle / PI * 180.0 );

			data.currWallHitboxes = &data.wallHitBody;

			data.latchedOn = false;
			data.basePos = myPos;
		}
		break;
	case RECOVER:
		//cout << "recover" << endl;
		break;
	}
}

void Gorilla::UpdateEnemyPhysics()
{
	V2d playerPos = sess->GetPlayerPos(0);
	if(data.latchedOn )
	{
		data.basePos = playerPos;
	}
	else
	{
		
		
	}

	V2d offsetDir = normalize(data.offsetPlayer );



	switch( action )
	{
	case WAKEUP:
		//frame = 0;
		break;
	case ALIGN:
		{
		/*double a = (double)physStepIndex / (dashFrames * NUM_MAX_STEPS * 5);
		if (a > 1.0)
		{
			action = S_OUT;
			frame = 0;
			physStepIndex = 0;
			break;
		}
		double f = dashBez.GetValue(a);
		double rf = 1.0 - f;

		position = currOrig * rf + dest * f;


		int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;*/

			CubicBezier b(0, 0, 1, 1);
			double f = b.GetValue(data.physStepIndex / ((double)data.alignMoveFrames * NUM_MAX_STEPS * 5));

			V2d idealOffset = offsetDir * idealRadius;
			
			
			data.offsetPlayer = data.origOffset * (1.0 - f) + idealOffset * f;
			//alignFrames += 5 / slowMultiple;
			int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;

			data.physStepIndex += steps;
		}
		//frame = 0;
		break;
	case FOLLOW:
		//cout << "follow: " << frame << endl;
		//frame = 0;
		break;
	case ATTACK:
		//cout << "ATTACK: " << frame << endl;
		//action = RECOVER;
		//frame = 0;
		break;
	case RECOVER:
		//cout << "recovery: " << frame << endl;
		//action = ALIGN;
		//frame = 0;
		break;
	}

	if(data.latchedOn )
	{
		currPosInfo.position = data.basePos + data.offsetPlayer;
	}
}


void Gorilla::UpdateSprite()
{
	if (data.latchedOn)
	{
		currPosInfo.position = data.basePos + data.offsetPlayer;
	}

	V2d diff = sess->GetPlayerPos(0) - GetPosition();
	double lenDiff = length(diff);
	IntRect ir;
	switch (action)
	{
	case WAKEUP:
		ir = ts->GetSubRect(0);
		break;
	case ALIGN:
		ir = ts->GetSubRect(0);
		break;
	case FOLLOW:
		ir = ts->GetSubRect(1);
		break;
	case ATTACK:
		ir = ts->GetSubRect(frame / animFactor[ATTACK] + 2);
		break;
	case RECOVER:
		ir = ts->GetSubRect(frame / animFactor[RECOVER] + 6);
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

	if ((action == ATTACK && frame > createWallFrame) || action == RECOVER)
	{
		target->draw(wallSprite);
	}
}

void Gorilla::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);

	if (data.currWallHitboxes != NULL)
		data.currWallHitboxes->DebugDraw(0, target);
}


bool Gorilla::CheckHitPlayer(int index)
{
	return BasicCheckHitPlayer(currHitboxes, index) || BasicCheckHitPlayer(data.currWallHitboxes, index);
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