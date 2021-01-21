#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_StagBeetle.h"
#include "Shield.h"

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

StagBeetle::StagBeetle( ActorParams *ap )
	:Enemy( EnemyType::EN_STAGBEETLE, ap ),
	moveBezTest( .22,.85,.3,.91 )
{
	SetLevel(ap->GetLevel());

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 11 * 5;
	actionLength[LAND] = 1;
	actionLength[JUMP] = 2;
	actionLength[RUN] = 9 * 4;
	
	gravity = .5;
	maxGroundSpeed = 10;
	maxFallSpeed = 40;


	const string &typeName = ap->GetTypeName();
	if (typeName == "reversestagbeetle")
	{
		reverse = true;
		sprite.setColor(Color::Blue);
		gravity = -gravity;
	}
	else
	{
		reverse = false;
		
	}
	
	attackMult = 3;

	//SetOffGroundHeight(176 / 2 * scale);
	//SetOffGroundHeight(200 * scale);

	CreateGroundMover(startPosInfo, 40, true, this);
	groundMover->AddAirForce(V2d(0, gravity));
	groundMover->SetSpeed( 0 );

	if (reverse)
	{
		groundMover->reverse = true;
	}

	ts_death = sess->GetTileset( "Enemies/stag_death_256x176.png", 256, 176 );
	ts_hop = sess->GetTileset( "Enemies/stag_hop_256x256.png", 256, 256 );
	ts_idle = sess->GetTileset( "Enemies/stag_idle_256x176.png", 256, 176 );
	ts_run = sess->GetTileset( "Enemies/stag_run_256x176.png", 256, 176);
	ts_sweep = sess->GetTileset( "Enemies/stag_sweep_256x298.png", 256, 298);
	ts_walk = sess->GetTileset( "Enemies/stag_walk_256x176.png", 256, 176 );

	ts_hopAura = sess->GetTileset("Enemies/stag_hop_aura_256x256.png", 256, 256);
	ts_idleAura = sess->GetTileset("Enemies/stag_idle_aura_256x176.png", 256, 176);
	ts_runAura = sess->GetTileset("Enemies/stag_run_aura_256x176.png", 256, 176);
	ts_sweepAura = sess->GetTileset("Enemies/stag_sweep_aura_256x298.png", 256, 298);
	ts_walkAura = sess->GetTileset("Enemies/stag_walk_aura_256x176.png", 256, 176);

	
	
	sprite.setTexture( *ts_idle->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;


	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	bezFrame = 0;
	bezLength = 60 * NUM_STEPS;

	cutObject->SetTileset(ts_death);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	cutObject->SetScale(scale);

	shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	ResetEnemy();
}

void StagBeetle::SetLevel(int lev)
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

void StagBeetle::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
		//testMover->physBody.DebugDraw(target);
}

void StagBeetle::ResetEnemy()
{
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	action = IDLE;

	facingRight = true;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	bezFrame = 0;
	attackFrame = -1;
	frame = 0;

	UpdateSprite();
	UpdateHitboxes();

	currShield = shield;
	shield->Reset();
	shield->SetPosition(GetPosition());
}

void StagBeetle::UpdateHitboxes()
{
	Edge *ground = groundMover->ground;
	if( ground != NULL )
	{

		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if(groundMover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15;
		}
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	BasicUpdateHitboxes();
}

void StagBeetle::ActionEnded()
{
	if( frame == actionLength[action] )
	{
		switch( action )
		{
		case IDLE:
			frame = 0;
			break;
		case RUN:
			frame = 0;
			break;
		case JUMP:
			frame = 1;
			break;
		case LAND:
			action = RUN;
			frame = 0;
			break;
		}
	}
}

bool StagBeetle::IsFacingTrueRight()
{
	return (facingRight && !reverse) || (!facingRight && reverse );
}

void StagBeetle::ProcessState()
{
	//cout << "vel: " << testMover->velocity.x << ", " << testMover->velocity.y << endl;
	//Actor *player = owner->GetPlayer( 0 );
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	ActionEnded();

	if( attackFrame == 11 * attackMult )
	{
		attackFrame = -1;
	}

	switch( action )
	{
	case IDLE:
		{
			double dist = length(playerPos - position );
			if( dist < 800 )
			{
				action = RUN;
				frame = 0;
				sprite.setTexture( *ts_run->texture );
			}
		}
		break;
	case RUN:
		{
			double dist = length(playerPos - position );
			if( dist >= 900 )
			{
				action = IDLE;
				frame = 0;
				sprite.setTexture( *ts_idle->texture );
			}
		}
		break;
	case JUMP:
		break;
	//case ATTACK:
	//	break;
	case LAND:
		break;
	}

	switch( action )
	{
	case IDLE:
		groundMover->SetSpeed( 0 );
		//cout << "idle: " << frame << endl;
		break;
	case RUN:
		//cout << "run: " << frame << endl;
		if(IsFacingTrueRight())
		{
			if(playerPos.x < position.x - 50 )
			{
				facingRight = !facingRight;
			}
		}
		else
		{
			if(playerPos.x > position.x + 50)
			{
				facingRight = !facingRight;
			}
		}

		if(facingRight) //clockwise
		{
			groundMover->SetSpeed(groundMover->groundSpeed + .3 );
		}
		else
		{
			groundMover->SetSpeed(groundMover->groundSpeed - .3 );
		}

		if(groundMover->groundSpeed > maxGroundSpeed )
			groundMover->SetSpeed( maxGroundSpeed );
		else if(groundMover->groundSpeed < -maxGroundSpeed )
			groundMover->SetSpeed( -maxGroundSpeed );
		break;
	case JUMP:
		//cout << "jump: " << frame << endl;
		break;
//	case ATTACK:
	//	{
	//		testMover->SetSpeed( 0 );
	//	}
	//	break;
	case LAND:
		{
		//	cout << "land: " << frame << endl;
			//testMover->SetSpeed( 0 );
		}
		break;
	default:
		//cout << "WAATATET" << endl;
		break;
	}
}

void StagBeetle::UpdateEnemyPhysics()
{
	if (numHealth > 0) //!dead
	{
		groundMover->Move(slowMultiple, numPhysSteps);

		if (groundMover->ground == NULL)
		{
			if (groundMover->velocity.y > maxFallSpeed)
			{
				groundMover->velocity.y = maxFallSpeed;
			}
			else if (groundMover->velocity.y < -maxFallSpeed)
			{
				groundMover->velocity.y = -maxFallSpeed;
			}
		}


		V2d gn(0, -1);
		if (groundMover->ground != NULL)
		{
			gn = groundMover->ground->Normal();
		}

		shield->SetPosition(GetPosition());
	}
}


void StagBeetle::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}


void StagBeetle::EnemyDraw(sf::RenderTarget *target )
{
	//target->draw(auraSprite);
	DrawSprite(target, sprite);// , auraSprite );
}



void StagBeetle::UpdateSprite()
{
	IntRect r;
	
	int attackOriginHeight = ts_sweep->tileHeight - ( 48 );
	//if( attackFrame >= 0 )
	//{
	//	sprite.setTexture( *ts_sweep->texture );
	//	r = ts_sweep->GetSubRect( attackFrame / attackMult );
	//	if( facingRight )
	//	{
	//		r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
	//	}
	//	sprite.setTextureRect( r );
	//	sprite.setOrigin( sprite.getLocalBounds().width / 2, attackOriginHeight);
	//	sprite.setRotation( angle );
	//	sprite.setPosition( gPoint.x, gPoint.y );
	//	//cout << "attacking angle " << angle << endl;
	//}
	//else
	{
		switch( action )
		{
		case IDLE:
		//	cout << "idle angle: " << angle << endl;
			//sprite.setTextureRect( *ts_idle->texture );
			sprite.setTexture( *ts_idle->texture );
			auraSprite.setTexture(*ts_idleAura->texture);
			r = ts_idle->GetSubRect( frame / 5 );

			break;
		case RUN:
			//cout << "run angle: " << angle << endl;
			//sprite.setTexture( *ts_run->texture );
			sprite.setTexture( *ts_run->texture );
			auraSprite.setTexture(*ts_runAura->texture);
			r = ts_run->GetSubRect( frame / 4 );
			break;
		case JUMP:
			{
				int tFrame = 0;
			sprite.setTexture( *ts_hop->texture );
			auraSprite.setTexture(*ts_hopAura->texture);
			//cout << "jump angle: " << angle << endl;

			if( frame == 0 )
			{

			}
			else
			{
				if(groundMover->velocity.y < 0 )
				{
					tFrame = 1;
				}
				else
					tFrame = 2;
			}
				
			r = ts_hop->GetSubRect( tFrame );
			
			break;
			}
		case LAND:
			sprite.setTexture( *ts_hop->texture );
			auraSprite.setTexture(*ts_hopAura->texture);
			r = ts_hop->GetSubRect( 3 );

			break;
		}

		if (( !facingRight && !reverse ) 
			|| ( reverse && groundMover->ground == NULL ) 
			|| ( reverse && !facingRight && groundMover->ground != NULL ) )
		{
			r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
		}

		if (reverse && groundMover->ground == NULL)
		{
			r = sf::IntRect(r.left, r.top + r.height, r.width, -r.height);
		}

		sprite.setTextureRect(r);

		float extraVert = 20 * scale;//8 * scale;

		int originHeight = 144 - (48);

		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 + extraVert);
		sprite.setPosition(GetPositionF());
		sprite.setRotation(groundMover->GetAngleDegrees());
	}

	SyncSpriteInfo(auraSprite, sprite);
}

void StagBeetle::HitTerrain( double &q )
{
	//cout << "hit terrain? " << endl;
}

bool StagBeetle::StartRoll()
{
	return false;
}

void StagBeetle::FinishedRoll()
{

}

void StagBeetle::HitOther()
{
	//cout << "hit other" << endl;
	
	if( action == RUN )
	{
	if( (facingRight && groundMover->groundSpeed < 0 )
		|| ( !facingRight && groundMover->groundSpeed > 0 ) )
	{
		//cout << "here" << endl;
		groundMover->SetSpeed( 0 );
	}
	else if( facingRight && groundMover->groundSpeed > 0 )
	{
		V2d v = V2d( maxGroundSpeed, -10 );
		groundMover->Jump( v );
		action = JUMP;
		frame = 0;
	}
	else if( !facingRight && groundMover->groundSpeed < 0 )
	{
		V2d v = V2d( -maxGroundSpeed, -10 );
		groundMover->Jump( v );
		action = JUMP;
		frame = 0;
	}
	}
	
	//cout << "hit other!" << endl;
	//testMover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void StagBeetle::ReachCliff()
{
	if( (facingRight && groundMover->groundSpeed < 0 )
		|| (!facingRight && groundMover->groundSpeed > 0) )
	{
		groundMover->SetSpeed( 0 );
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;

	double jumpStrength = -10;
	if (reverse)
		jumpStrength = -jumpStrength;

	V2d v;
	if(IsFacingTrueRight() )
	{
		v = V2d( maxGroundSpeed, jumpStrength);
	}
	else
	{
		v = V2d( -maxGroundSpeed, jumpStrength);
	}

	groundMover->Jump( v );

	action = JUMP;
	frame = 0;

	//cout << "Reach cliff" << endl;
	//testMover->groundSpeed = -testMover->groundSpeed;
	//facingRight = !facingRight;
}

void StagBeetle::HitOtherAerial( Edge *e )
{
	//cout << "hit edge" << endl;
}

void StagBeetle::Land()
{
	action = LAND;
	//sprite.setTexture( *ts_hop->texture );
	//sprite.setTextureRect( ts_hop->GetSubRect( 0 ) );
	frame = 0;
	//cout << "land" << endl;
}