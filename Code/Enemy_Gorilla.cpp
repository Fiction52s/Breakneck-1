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

	latchedOn = false;

	approachFrames = 180 * 3;
	totalFrame = 0;

	animationFactor = 5;

	//ts = owner->GetTileset( "Gorilla.png", 80, 80 );
	ts = sess->GetSizedTileset( "Enemies/gorilla_320x256.png");

	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);

	hitBody.hitboxInfo = hitboxInfo;

	wallHitboxWidth = wallWidth;
	wallHitboxHeight = 50;

	wallHitboxInfo = new HitboxInfo;
	wallHitboxInfo->damage = 18;
	wallHitboxInfo->drainX = 0;
	wallHitboxInfo->drainY = 0;
	wallHitboxInfo->hitlagFrames = 0;
	wallHitboxInfo->hitstunFrames = 10;
	wallHitboxInfo->knockback = 4;

	wallHitBody.SetupNumFrames(1);
	wallHitBody.SetupNumBoxesOnFrame(0, 1);
	wallHitBody.AddBasicRect(0, wallHitboxWidth/2, wallHitboxHeight/2, 0, V2d());
	
	wallHitBody.hitboxInfo = wallHitboxInfo;
	

	ts_wall = sess->GetSizedTileset("Enemies/gorillawall_400x50.png");
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

	latchStartAngle = 0;
	latchedOn = false;
	totalFrame = 0;
	currWallHitboxes = NULL;
	
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	dead = false;
	frame = 0;
	basePos = startPosInfo.GetPosition();
	
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
			alignFrames = 0;
			frame = 0;
			physStepIndex = 0;
			if (playerPos.x < GetPosition().x)
			{
				facingRight = false;
			}
			else
			{
				facingRight = true;
			}

			latchedOn = true;
			offsetPlayer = basePos - playerPos;
			origOffset = offsetPlayer;
			V2d offsetDir = normalize(offsetPlayer);

			basePos = playerPos;

			double currRadius = length(offsetPlayer);
			alignMoveFrames = 60;// *5 * NUM_STEPS;
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
				latchedOn = true;
				offsetPlayer = basePos - playerPos;
				origOffset = offsetPlayer;
				V2d offsetDir = normalize( offsetPlayer );

				basePos = playerPos;
				currWallHitboxes = NULL;

				action = ALIGN;
				double currRadius = length( offsetPlayer );
				alignMoveFrames = 60;
				frame = 0;
				physStepIndex = 0;
				alignFrames = 0;

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
			if( alignFrames >= alignMoveFrames )
			{
				action = FOLLOW;
				frame = 0;
			}
			else
			{
				++alignFrames;
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
	
			V2d playerDir = -normalize( origOffset );

			CollisionBox &wallHitbox = wallHitBody.GetCollisionBoxes(0).front();

			wallHitbox.globalPosition = myPos + playerDir * wallAmountCloser;
			wallHitbox.globalAngle = atan2( playerDir.x, -playerDir.y );

			wallSprite.setPosition( wallHitbox.globalPosition.x, 
				wallHitbox.globalPosition.y );
			wallSprite.setRotation( wallHitbox.globalAngle / PI * 180.0 );

			currWallHitboxes = &wallHitBody;

			latchedOn = false;
			basePos = myPos;
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
	if( latchedOn )
	{
		basePos = playerPos;
	}
	else
	{
		
		
	}

	V2d offsetDir = normalize( offsetPlayer );



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
			double f = b.GetValue(physStepIndex / ((double)alignMoveFrames * NUM_MAX_STEPS * 5));

			V2d idealOffset = offsetDir * idealRadius;
			
			
			offsetPlayer = origOffset * (1.0 - f) + idealOffset * f;
			//alignFrames += 5 / slowMultiple;
			int steps = (5 / slowMultiple) * NUM_MAX_STEPS / numPhysSteps;

			physStepIndex += steps;
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

	if( latchedOn )
	{
		currPosInfo.position = basePos + offsetPlayer;
	}
}


void Gorilla::UpdateSprite()
{
	if (latchedOn)
	{
		currPosInfo.position = basePos + offsetPlayer;
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

	if (currWallHitboxes != NULL)
		currWallHitboxes->DebugDraw(0, target);
}


bool Gorilla::CheckHitPlayer(int index)
{
	return BasicCheckHitPlayer(currHitboxes, index) || BasicCheckHitPlayer(currWallHitboxes, index);
}