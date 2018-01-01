#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Crawler.h"

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

Crawler::Crawler( GameSession *owner, bool p_hasMonitor, Edge *g, double q, bool cw, int s, int p_framesUntilBurrow )
	:Enemy( owner, EnemyType::EN_CRAWLER, p_hasMonitor, 1 ), ground( g ), edgeQuantity( q ), clockwise( cw ), groundSpeed( s )
{
	origCW = cw;
	numHealth = 4;
	//clockwise = cw;
	maxFramesUntilBurrow = p_framesUntilBurrow;
	maxFramesUntilBurrow = 200;
	framesUntilBurrow = maxFramesUntilBurrow;

	dashAccel = 3;
	currDistTravelled = 0;
	mover = new SurfaceMover(owner, g, q, 32 );
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	initHealth = 60;
	health = initHealth;
	dead = false;
	deathFrame = 0;
	attackFrame = -1;
	attackMult = 3;

	double height = 128;
	double width = 128;
	ts = owner->GetTileset( "crawler_128x128.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = mover->ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	V2d gNorm = mover->ground->Normal();

	//30 is front
	//cutObject->ts = ts;
	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(61);
	cutObject->SetSubRectBack(62);
	
	

	double angle = atan2( gNorm.x, -gNorm.y );
	sprite.setRotation( angle / PI * 180.f );
	roll = false;
	position = gPoint + gNorm * height / 2.0;
	
	//receivedHit = NULL;

	double size = max( width, height );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size/ 2, size, size );


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox( 0, hurtBox);
	
	
	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody->AddCollisionBox(0, hitBox);
	hitBody->hitboxInfo = hitboxInfo;


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	crawlAnimationFactor = 2;
	rollAnimationFactor = 5;
	//mover->physBody.isCircle = true;
	//mover->physBody.offset.x = 0;
	/*physBody.offset.y = 0;
	physBody.rw = 32;
	physBody.rh = 32;
	physBody.type = CollisionBox::BoxType::Physics;*/

	startGround = ground;
	startQuant = edgeQuantity;
	frame = 0;

	deathPartingSpeed = .4;
	//mover->SetSpeed(groundSpeed);
	//action = CRAWL;

	actionLength[UNBURROW] = 20;
	actionLength[CRAWL] = 35 * crawlAnimationFactor;
	actionLength[STARTROLL] = 3 * crawlAnimationFactor;
	actionLength[ROLL] = 13 * crawlAnimationFactor;
	actionLength[ENDROLL] = 4 * crawlAnimationFactor;
	actionLength[DASH] = 7 * crawlAnimationFactor;
	actionLength[BURROW] = 20;//3 * crawlAnimationFactor;
	actionLength[UNDERGROUND] = 20;//3 * crawlAnimationFactor;
	//actionLength[DYING] = 1;//3 * crawlAnimationFactor;

	action = UNDERGROUND;
	frame = actionLength[UNDERGROUND];
}

void Crawler::ResetEnemy()
{
	clockwise = origCW;
	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
	numHealth = 4;
	framesUntilBurrow = maxFramesUntilBurrow;
	
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);
	//mover->SetSpeed(groundSpeed);

	position = mover->physBody.globalPosition;

	currDistTravelled = 0;
	health = initHealth;
	attackFrame = -1;
	roll = false;
	ground = startGround;
	edgeQuantity = startQuant;
	V2d gPoint = ground->GetPoint( edgeQuantity );
	sprite.setPosition( gPoint.x, gPoint.y );
	frame = 0;
	position = gPoint + ground->Normal() * 64.0 / 2.0;
	V2d gn = mover->ground->Normal();
	/*
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;*/

	//position = gPoint + offset;

	deathFrame = 0;
	dead = false;

	//----update the sprite
	double angle = 0;
	angle = atan2( gn.x, -gn.y );
		
	//sprite.setTexture( *ts_walk->texture );
	sprite.setTextureRect( ts->GetSubRect( frame / crawlAnimationFactor ) );
	V2d pp = mover->ground->GetPoint(mover->edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( pp.x, pp.y );

	UpdateHitboxes();

	action = UNDERGROUND;
	frame = actionLength[UNDERGROUND];
}

void Crawler::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	if( mover->ground != NULL )
	{
		V2d gn = ground->Normal();
		double angle = 0;
		
		
		angle = atan2( gn.x, -gn.y );
		
		hitBox.globalAngle = angle;
		hurtBox.globalAngle = angle;

		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		double maxExtraKB = 15.0;
		if( mover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15 + max( abs( mover->groundSpeed )/2, maxExtraKB);
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15 + max(abs(mover->groundSpeed)/2, maxExtraKB);
		}
	}
	else
	{
		hitBox.globalAngle = 0;
		hurtBox.globalAngle = 0;
	}
	hitBox.globalPosition = mover->physBody.globalPosition;
	hurtBox.globalPosition = mover->physBody.globalPosition;
	//mover->ph.globalPosition = mover->physBody.globalPosition;
}

void Crawler::ProcessState()
{
	V2d en = mover->ground->Normal();

	if (action != BURROW && action != UNDERGROUND)
	{
		if (framesUntilBurrow == 0)
		{
			action = BURROW;
			frame = 0;
			mover->SetSpeed(0);
		}
	}

	if (frame == actionLength[action])
		//if( false )
	{
		switch (action)
		{
		case UNBURROW:
			action = CRAWL;
			frame = 0;
			if (clockwise)
			{
				mover->SetSpeed(groundSpeed);
			}
			else
			{
				mover->SetSpeed(-groundSpeed);
			}
			break;
		case CRAWL:
			frame = 0;
			break;
		case STARTROLL:
			action = ROLL;
			frame = 0;
			break;
		case ROLL:
			frame = 0;
			break;
		case ENDROLL:
			action = CRAWL;
			frame = 0;
			break;
		case DASH:
			if (mover->groundSpeed > 0 && (en.x > 0 && en.y < 0)
				|| (mover->groundSpeed < 0 && en.x < 0 && en.y < 0))
			{
				action = STARTROLL;
				frame = 0;
			}
			else
			{
				action = CRAWL;
				frame = 0;
				//mover->SetSpeed(groundSpeed);
			}
			break;
		case BURROW:
			action = UNDERGROUND;
			frame = 0;
			mover->ground = startGround;
			mover->edgeQuantity = startQuant;
			mover->roll = false;
			mover->UpdateGroundPos();
			break;
		case UNDERGROUND:
			action = UNBURROW;
			framesUntilBurrow = maxFramesUntilBurrow;
			frame = 0;
			if (!PlayerInFront())
			{
				clockwise = !clockwise;
			}
			break;
		}
	}

	switch (action)
	{
	case UNBURROW:
		break;
	case CRAWL:
		break;
	case DASH:
		break;
	case BURROW:
		break;
	}


	switch (action)
	{
	case UNBURROW:
		break;
	case CRAWL:
		TryDash();
		AttemptRunAwayBoost();
		break;
	case STARTROLL:
		TryDash();
		AttemptRunAwayBoost();
		break;
	case ROLL:
		TryDash();
		{
			V2d gn = mover->ground->Normal();
			double acc = .08;

			if ((mover->groundSpeed > 0 && gn.x < 0) || (mover->groundSpeed < 0 && gn.x > 0))
			{
				Accelerate(acc);
			}
		}
		AttemptRunAwayBoost();
		break;
	case ENDROLL:
		AttemptRunAwayBoost();
		break;
	case DASH:
		AttemptRunAwayBoost();
		break;
	case BURROW:
		break;
	}
}

void Crawler::UpdateEnemyPhysics()
{
	if (!dead)
	{
		mover->Move(slowMultiple);
		position = mover->physBody.globalPosition;
	}
}

void Crawler::FrameIncrement()
{
	if (framesUntilBurrow > 0)
		--framesUntilBurrow;
}

void Crawler::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		if (action != UNBURROW && action != BURROW && action != UNDERGROUND)
		{
			if (hasMonitor && !suppressMonitor)
			{
				if (owner->pauseFrames < 2 || receivedHit == NULL)
				{
					target->draw(sprite, keyShader);
				}
				else
				{
					target->draw(sprite, hurtShader);
				}
				target->draw(*keySprite);
			}
			else
			{
				if (owner->pauseFrames < 2 || receivedHit == NULL)
				{
					target->draw(sprite);
				}
				else
				{
					target->draw(sprite, hurtShader);
				}

			}
		}
		else
		{
			if (action != UNDERGROUND)
			{
				sf::RectangleShape rs;
				rs.setFillColor(Color::Red);
				rs.setSize(Vector2f(64, 64));
				rs.setOrigin(rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2);
				rs.setPosition(Vector2f(mover->physBody.globalPosition));
				target->draw(rs);
			}
		}
	}
	else
	{
		//target->draw( botDeathSprite );
		cutObject->Draw(target);
		//target->draw( topDeathSprite );
	}
}

void Crawler::IHitPlayer( int index )
{
	
}

void Crawler::HandleNoHealth()
{
	
	//cutObject->flipHoriz = !clockwise;
	cutObject->SetFlipHoriz(!clockwise);
	cutObject->rotateAngle = sprite.getRotation();
	//cutObject->SetCutRootPos(Vector2f(position.x, position.y));
	//action = DYING;
	//dead = true;
	//frame = 0;
	//int x = 5;
}

void Crawler::UpdateSprite()
{
	if( !dead )
	{
		//cout << "response" << endl;
		double spaceNeeded = 0;
		V2d gn = mover->ground->Normal();
		V2d gPoint = mover->ground->GetPoint(mover->edgeQuantity);

		//return;

		double angle = 0;

		IntRect ir;
		switch (action)
		{
		case CRAWL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor);
			break;
		case STARTROLL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 35);
			break;
		case ROLL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 38 );
			break;
		case ENDROLL:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 50);
			break;
		case DASH:
			ir = ts->GetSubRect(frame / crawlAnimationFactor + 54);
			break;
		}

		if (!clockwise)
		{
			sprite.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
		}
		else
		{
			sprite.setTextureRect(ir);
		}

		if (!mover->roll)
		{
			angle = atan2(gn.x, -gn.y);

			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation(angle / PI * 180);
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			if (clockwise)
			{
				V2d vec = normalize(position - mover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
				sprite.setPosition(pp.x, pp.y);
			}
			else
			{
				V2d vec = normalize(position - mover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation(angle / PI * 180);
				V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
				sprite.setPosition(pp.x, pp.y);
			}
		}
	}
}

void Crawler::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	if (!dead)
		mover->physBody.DebugDraw(target);
}

void Crawler::TransferEdge(Edge *e)
{
	V2d en = e->Normal();
	if ( mover->groundSpeed > 0 && (en.x > 0 && en.y < 0) 
		|| ( mover->groundSpeed < 0 && en.x < 0 && en.y < 0))
	{
		if (action != ROLL && action != DASH )
		{
			action = ROLL;
			frame = 0;
		}
	}
	else if (action == ROLL)
	{
		action = ENDROLL;
		frame = 0;
	}
}

bool Crawler::ShouldDash()
{
	V2d dir;
	if (clockwise)
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise)
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double heightOff = cross(owner->GetPlayer(0)->position - mover->physBody.globalPosition, dir);
	if (abs(heightOff) > 150 )
		return false;

	if (length(owner->GetPlayer(0)->position - position) < 350)
	{
		if (PlayerInFront())
			return true;
	}
	return false;
}

bool Crawler::PlayerInFront()
{
	V2d dir;
	if (clockwise )
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise )
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double alongDist = dot(owner->GetPlayer(0)->position - mover->physBody.globalPosition, dir);
	if (alongDist > -60)
		return true;
	else
		return false;
}

void Crawler::Accelerate(double amount)
{
	if ( clockwise )
	{
		amount = abs(amount);
	}
	else
	{
		amount = -abs(amount);
	}
	mover->SetSpeed( mover->groundSpeed + amount );
}

void Crawler::SetForwardSpeed( double speed )
{
	double aSpeed = abs(speed);
	if (clockwise)
	{
		mover->SetSpeed(aSpeed);
	}
	else
	{
		mover->SetSpeed(-aSpeed);
	}
}

void Crawler::SetActionDash()
{
	action = DASH;
	frame = 0;

	Accelerate(dashAccel);
}

bool Crawler::TryDash()
{
	if (ShouldDash())
	{
		SetActionDash();
		return true;
	}

	return false;
}

bool Crawler::IsPlayerChasingMe()
{
	return (!PlayerInFront() &&
		length(owner->GetPlayer(0)->position - mover->physBody.globalPosition) < 400);
}

void Crawler::AttemptRunAwayBoost()
{
	if (IsPlayerChasingMe())
	{
		Accelerate(.1);
	}
}