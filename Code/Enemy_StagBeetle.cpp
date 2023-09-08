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
	:Enemy( EnemyType::EN_STAGBEETLE, ap )
{
	SetLevel(ap->GetLevel());

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 11 * 5;
	actionLength[LAND] = 1;
	actionLength[JUMP] = 2;
	actionLength[RUN] = 9 * 4;
	
	origGravity = .5;
	gravity = origGravity;
	maxGroundSpeed = 10;
	maxFallSpeed = 40;

	reverse = false;
	
	attackMult = 3;

	//SetOffGroundHeight(176 / 2 * scale);
	//SetOffGroundHeight(200 * scale);

	CreateGroundMover(startPosInfo, 40, true, this);
	
	groundMover->SetSpeed( 0 );

	if (reverse)
	{
		groundMover->SetReverse(true);
	}

	ts_death = GetSizedTileset("Enemies/W2/stag_death_192x176.png");
	ts_hop = GetSizedTileset( "Enemies/W2/stag_hop_256x256.png");
	ts_idle = GetSizedTileset( "Enemies/W2/stag_idle_256x176.png");
	ts_run = GetSizedTileset( "Enemies/W2/stag_run_256x176.png");
	ts_sweep = GetSizedTileset( "Enemies/W2/stag_sweep_256x298.png");
	ts_walk = GetSizedTileset( "Enemies/W2/stag_walk_256x176.png");
	
	
	sprite.setTexture( *ts_idle->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 5;
	hitboxInfo->hitstunFrames = 40;
	hitboxInfo->knockback = 20;
	hitboxInfo->hType = HitboxInfo::GREEN;
	

	BasicCircleHurtBodySetup(60);
	BasicCircleHitBodySetup(60);
	hitBody.hitboxInfo = hitboxInfo;

	shield = NULL;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	cutObject->SetTileset(ts_death);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	cutObject->SetScale(scale);

	//shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	ResetEnemy();
}

StagBeetle::~StagBeetle()
{
	//delete shield;
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
}

void StagBeetle::ResetEnemy()
{
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);
	groundMover->ClearAirForces();

	Edge *e = startPosInfo.GetEdge();
	if( e != NULL && e->Normal().y > 0 )
	{
		reverse = true;
		sprite.setColor(Color::Blue);
		gravity = -origGravity;

		if (PlayerDir().x >= 0)
		{
			facingRight = false;
		}
		else
		{
			facingRight = true;
		}
	}
	else
	{
		reverse = false;
		sprite.setColor(Color::White);
		gravity = origGravity;

		if (PlayerDir().x >= 0)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
	}

	groundMover->AddAirForce(V2d(0, gravity));


	action = IDLE;

	
	

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	frame = 0;

	UpdateSprite();
	UpdateHitboxes();

	//currShield = shield;
	//shield->Reset();
	//shield->SetPosition(GetPosition());
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
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	ActionEnded();

	//if(data.attackFrame == 11 * attackMult )
	//{
	//	data.attackFrame = -1;
	//}

	switch( action )
	{
	case IDLE:
		{
			double dist = length(playerPos - position );
			if( dist < DEFAULT_DETECT_RADIUS)
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
			if( dist >= DEFAULT_IGNORE_RADIUS )
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
			groundMover->SetSpeed(groundMover->GetGroundSpeed() + .3 );
		}
		else
		{
			groundMover->SetSpeed(groundMover->GetGroundSpeed() - .3 );
		}

		if(groundMover->GetGroundSpeed() > maxGroundSpeed )
			groundMover->SetSpeed( maxGroundSpeed );
		else if(groundMover->GetGroundSpeed() < -maxGroundSpeed )
			groundMover->SetSpeed( -maxGroundSpeed );
		break;
	case JUMP:
		if (facingRight)
		{
			groundMover->SetVelX(maxGroundSpeed);
		}
		else
		{
			groundMover->SetVelX(-maxGroundSpeed);
		}
		//cout << "jump: " << frame << endl;
		break;
	case LAND:
		{
		//	cout << "land: " << frame << endl;
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
			if (groundMover->GetVel().y > maxFallSpeed)
			{
				groundMover->SetVelY(maxFallSpeed);
			}
			else if (groundMover->GetVel().y < -maxFallSpeed)
			{
				groundMover->SetVelY(-maxFallSpeed);
			}
		}


		V2d gn(0, -1);
		if (groundMover->ground != NULL)
		{
			gn = groundMover->ground->Normal();
		}

		//shield->SetPosition(GetPosition());
	}
}


void StagBeetle::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->rotateAngle = sprite.getRotation();
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
			r = ts_idle->GetSubRect( frame / 5 );

			break;
		case RUN:
			//cout << "run angle: " << angle << endl;
			//sprite.setTexture( *ts_run->texture );
			sprite.setTexture( *ts_run->texture );
			r = ts_run->GetSubRect( frame / 4 );
			break;
		case JUMP:
			{
				int tFrame = 0;
			sprite.setTexture( *ts_hop->texture );
			//cout << "jump angle: " << angle << endl;

			if( frame == 0 )
			{

			}
			else
			{
				
				if( (!reverse && groundMover->GetVel().y < 0) || 
					(reverse && groundMover->GetVel().y > 0 ))
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
			r = ts_hop->GetSubRect( 3 );

			break;
		}

		if (( !facingRight && !reverse ) 
			|| ( reverse && facingRight && groundMover->ground == NULL ) 
			|| ( reverse && !facingRight && groundMover->ground != NULL ) )
		{
			r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
		}

		if (reverse && groundMover->ground == NULL)
		{
			r = sf::IntRect(r.left, r.top + r.height, r.width, -r.height);
		}

		sprite.setTextureRect(r);

		//when you scale this its too large. no idea why
		float extraVert = 20;//20;// *scale;//8 * scale;

		//extraVert += 2 * (scale-1);

		int originHeight = 144 - (48);

		//if (groundMover->ground != NULL)
		//{
		//	sprite.setRotation(groundMover->GetAngleDegrees());
		//	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		//	sprite.setPosition(groundMover->GetGroundPointF());
		//}
		//else
		//{
		//	sprite.setRotation(0);
		//	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 + 32);
		//	sprite.setPosition(GetPositionF());
		//}

		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 + extraVert);
		sprite.setPosition(GetPositionF());
		////sprite.setPosition(groundMover->GetGroundPointF());
		sprite.setRotation(groundMover->GetAngleDegrees());
	}
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
	if( (facingRight && groundMover->GetGroundSpeed() < 0 )
		|| ( !facingRight && groundMover->GetGroundSpeed() > 0 ) )
	{
		//cout << "here" << endl;
		groundMover->SetSpeed( 0 );
	}
	else if( facingRight && groundMover->GetGroundSpeed() > 0 )
	{
		V2d v = V2d( maxGroundSpeed, -10 );
		groundMover->Jump( v );
		action = JUMP;
		frame = 0;
	}
	else if( !facingRight && groundMover->GetGroundSpeed() < 0 )
	{
		V2d v = V2d( -maxGroundSpeed, -10 );
		groundMover->Jump( v );
		action = JUMP;
		frame = 0;
	}
	}
}

void StagBeetle::ReachCliff()
{
	if( (facingRight && groundMover->GetGroundSpeed() < 0 )
		|| (!facingRight && groundMover->GetGroundSpeed() > 0) )
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

	//Edge *ground = groundMover->ground;
	//cout << "land" << endl;
}

void StagBeetle::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	if (reverse)
	{
		hitboxInfo->kbDir = normalize(V2d(1, .7));
	}
	else
	{
		hitboxInfo->kbDir = normalize(V2d(1, -.7));
	}
}

int StagBeetle::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void StagBeetle::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void StagBeetle::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}