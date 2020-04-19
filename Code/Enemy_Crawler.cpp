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

Enemy *Crawler::Create(ActorParams *ap)
{
	return new Crawler(ap->hasMonitor, ap->GetGroundEdge(), ap->groundInfo->groundQuantity, ap->enemyLevel);
}

Crawler::Crawler(bool p_hasMonitor, Edge *g, double q, int p_level )
	:Enemy( EnemyType::EN_CRAWLER, p_hasMonitor, 1 ), clockwise( true ), groundSpeed( 5 )
{
	level = p_level;

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


	origCW = true;
	//maxFramesUntilBurrow = p_framesUntilBurrow;
	maxFramesUntilBurrow = 200;
	framesUntilBurrow = maxFramesUntilBurrow;

	dashAccel = .1;
	currDistTravelled = 0;
	mover = new SurfaceMover(g, q, 32 * scale );
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	dead = false;

	deathSound = sess->GetSound("Enemies/crawler_death");

	double height = 160;
	double width = 160;
	ts = sess->GetTileset( "Enemies/crawler_160x160.png", width, height );
	ts_aura = sess->GetTileset("Enemies/crawler_aura_160x160.png", width, height);

	height *= scale;
	width *= scale;

	auraSprite.setTexture(*ts_aura->texture);
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = mover->ground->GetPoint( q );
	sprite.setPosition( gPoint.x, gPoint.y );

	sprite.setScale(scale, scale);
	V2d gNorm = mover->ground->Normal();

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(86);
	cutObject->SetSubRectBack(87);
	cutObject->SetScale(scale);
	
	double angle = atan2( gNorm.x, -gNorm.y );
	sprite.setRotation( angle / PI * 180.f );
	//position = gPoint + gNorm * height / 2.0;
	position = mover->physBody.globalPosition;

	double size = max( width, height );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size/ 2, size, size );

	animFactor[UNBURROW] = 4;
	animFactor[CRAWL] = 1;
	animFactor[ATTACK] = 4;
	animFactor[ROLL] = 4;
	animFactor[DASH] = 1;
	animFactor[BURROW] = 4;
	animFactor[UNDERGROUND] = 1;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 6;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(32, position);
	BasicCircleHitBodySetup(32, position);

	
	hitBody.hitboxInfo = hitboxInfo;

	crawlAnimationFactor = 2;
	rollAnimationFactor = 5;

	startGround = g;
	startQuant = q;
	frame = 0;


	
	actionLength[UNBURROW] = 20;
	actionLength[CRAWL] = 35;
	actionLength[ROLL] = 5;
	actionLength[DASH] = 7;
	actionLength[ATTACK] = 6;
	actionLength[BURROW] = 19;//3 * crawlAnimationFactor;
	actionLength[UNDERGROUND] = 20;//3 * crawlAnimationFactor;
	//actionLength[DYING] = 1;//3 * crawlAnimationFactor;

	//action = UNDERGROUND;
	//frame = actionLength[UNDERGROUND];
	ResetEnemy();
}

Crawler::~Crawler()
{
	delete mover;
}

void Crawler::PlayDeathSound()
{
	Enemy::PlayDeathSound();
	sess->ActivateSoundAtPos( position, deathSound);
}



void Crawler::ResetEnemy()
{
	clockwise = origCW;
	SetHurtboxes(NULL, 0);
	SetHitboxes(NULL, 0);
	framesUntilBurrow = maxFramesUntilBurrow;
	
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);

	position = mover->physBody.globalPosition;

	currDistTravelled = 0;

	frame = 0;
	V2d gPoint = mover->ground->GetPoint( startQuant );
	//sprite.setPosition( gPoint.x, gPoint.y );
	
	//position = gPoint + mover->ground->Normal() * 64.0 / 2.0;
	V2d gn = mover->ground->Normal();
	dead = false;

	double angle = 0;
	angle = atan2( gn.x, -gn.y );

	//sprite.setTextureRect( ts->GetSubRect( frame / crawlAnimationFactor ) );
	//V2d pp = mover->ground->GetPoint(mover->edgeQuantity );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	//sprite.setRotation( angle / PI * 180 );
	//sprite.setPosition( pp.x, pp.y );

	UpdateHitboxes();

	action = UNBURROW;
	frame = 15 * 4;

	//SetHitboxes(hitBody, 0);
	//SetHurtboxes(hurtBody, 0);
	framesUntilBurrow = maxFramesUntilBurrow;
	
	DecideDirection();

	UpdateSprite();
}

void Crawler::DecideDirection()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d gn = mover->ground->Normal();
	if (gn.y < 0)
	{
		if (playerPos.x < position.x)
		{
			clockwise = false;
		}
		else
		{
			clockwise = true;
		}
	}
	else if (gn.y > 0)
	{
		if (playerPos.x < position.x)
		{
			clockwise = true;
		}
		else
		{
			clockwise = false;
		}
	}
	else
	{
		if (gn.x < 0)
		{
			if (playerPos.y < position.y)
			{
				clockwise = false;
			}
			else
			{
				clockwise = true;
			}
		}
		else if (gn.x > 0)
		{
			if (playerPos.y > position.y)
			{
				clockwise = true;
			}
			else
			{
				clockwise = false;
			}
		}
	}
}

void Crawler::UpdateHitboxes()
{
	BasicUpdateHitboxes();
	
	if( mover->ground != NULL )
	{
		V2d gn = mover->ground->Normal();
		double angle = 0;
		
		
		angle = atan2( gn.x, -gn.y );
		
		//hitBox.globalAngle = angle;
		//hurtBox.globalAngle = angle;

		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		double maxExtraKB = 15.0;
		double baseKB = 8;
		double scaleFactor = .2;
		double sc = scale * scaleFactor - scaleFactor;
		maxExtraKB += maxExtraKB * sc;
		baseKB += baseKB * sc;
		if( mover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = baseKB + max( abs( mover->groundSpeed ), maxExtraKB);
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = baseKB + max(abs(mover->groundSpeed), maxExtraKB);
		}
	}
	else
	{
		//hitBox.globalAngle = 0;
		//hurtBox.globalAngle = 0;
	}
	
}

void Crawler::ProcessState()
{
	if (mover->ground->edgeType == Edge::OPEN_GATE)
	{
		DirectKill();
		return;
	}

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

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case UNBURROW:
			action = DECIDE;
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
		case ATTACK:
			action = DECIDE;
			frame = 0;
			break;
		case ROLL:
			frame = 0;
			break;
		case DASH:
			action = DECIDE;
			frame = 0;
			break;
		case BURROW:
			SetHurtboxes(NULL, 0);
			action = UNDERGROUND;
			frame = 0;
			mover->ground = startGround;
			mover->edgeQuantity = startQuant;
			mover->roll = false;
			mover->UpdateGroundPos();
			break;
		case UNDERGROUND:
			SetHurtboxes(&hurtBody, 0);
			action = UNBURROW;
			framesUntilBurrow = maxFramesUntilBurrow;
			frame = 0;
			DecideDirection();
			break;
		}
	}

	switch (action)
	{
	case UNBURROW:
		if (frame == 4 * 11)
		{
			//SetHitboxes(&hurtBody, 0);
			SetHitboxes(&hitBody, 0);
		}
		break;
	case CRAWL:
		break;
	case DASH:
		break;
	case BURROW:
		if (frame == 4 * 12)
		{
			SetHitboxes(NULL, 0);
			SetHurtboxes(NULL, 0);
		}
		break;
	}

	if (action == DECIDE)
	{
		DecideMovement();
	}

	TryDashAndAttack();
	AttemptRunAwayBoost();

	switch (action)
	{
	case ROLL:
	{
		V2d gn = mover->ground->Normal();
		double rollacc = .08;

		if ((mover->groundSpeed > 0 && gn.x < 0 && gn.y < 0) || (mover->groundSpeed < 0 && gn.x > 0 && gn.y < 0))
		{
			Accelerate(rollacc);
		break;
		}
	}
	case DASH:
		Accelerate(dashAccel);
		break;
	case ATTACK:
		Accelerate(dashAccel);
		break;
	}

	//cout << "action: " << action << ", frame: " << frame << endl;
	/*switch (action)
	{
	case UNBURROW:
		break;
	case CRAWL:
		TryDash();
		
		break;
	case STARTROLL:
		TryDash();
		AttemptRunAwayBoost();
		break;
	case ROLL:
		TryDash();
		{
			
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
	}*/
}

void Crawler::UpdateEnemyPhysics()
{
	if (numHealth > 0 ) //!dead
	{
		mover->Move(slowMultiple, numPhysSteps );
		position = mover->physBody.globalPosition;
	}
}

void Crawler::FrameIncrement()
{
	if (framesUntilBurrow > 0)
		--framesUntilBurrow;
}

void Crawler::EnemyDraw(sf::RenderTarget *target )
{
	if (action == UNDERGROUND)
		return; 
	target->draw(auraSprite);
	DrawSpriteIfExists(target, sprite);
}

void Crawler::IHitPlayer( int index )
{
	
}

void Crawler::DecideMovement()
{
	V2d en = mover->ground->Normal();
	if (mover->groundSpeed > 0 && (en.x > 0 && en.y < 0)
		|| (mover->groundSpeed < 0 && en.x < 0 && en.y < 0))
	{
		if (action != ROLL)
		{
			action = ROLL;
			frame = 0;
		}
	}
	else
	{
		if (action != CRAWL)
		{
			action = CRAWL;
			frame = 0;
		}
	}
}

bool Crawler::TryDashAndAttack()
{
	if (TryAttack())
	{
		return true;
	}

	return TryDash();
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
	V2d gn = mover->ground->Normal();
	V2d gPoint = mover->ground->GetPoint(mover->edgeQuantity);

	//return;
	float extraVert = 34;

	double angle = 0;

	IntRect ir;
	switch (action)
	{
	case CRAWL:
		ir = ts->GetSubRect(frame / animFactor[CRAWL]);
		break;
	case ROLL:
		ir = ts->GetSubRect(frame / animFactor[ROLL]+ 35 );
		break;
	case DASH:
		ir = ts->GetSubRect(79);
		break;
	case BURROW:
		ir = ts->GetSubRect(frame / animFactor[BURROW] + 40);
		break;
	case UNBURROW:
		ir = ts->GetSubRect(frame / animFactor[UNBURROW] + 59);
		break;
	case ATTACK:
		ir = ts->GetSubRect(frame / animFactor[ATTACK] + 80);
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
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
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

			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
			sprite.setRotation(angle / PI * 180);
			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			V2d vec = normalize(position - mover->ground->v0);
			angle = atan2(vec.y, vec.x);
			angle += PI / 2.0;

			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
			sprite.setRotation(angle / PI * 180);
			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
			sprite.setPosition(pp.x, pp.y);
		}
	}

	SyncSpriteInfo(auraSprite, sprite);
}

void Crawler::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
	//	mover->physBody.DebugDraw(target);
}

void Crawler::TransferEdge(Edge *e)
{
	DecideMovement();
}

bool Crawler::ShouldAttack()
{
	if (action != CRAWL && action != ROLL && action != DASH && action != DECIDE )
	{
		return false;
	}

	V2d playerPos = sess->GetPlayerPos(0);

	V2d dir;
	if (clockwise)
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise)
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double heightOff = cross(playerPos - mover->physBody.globalPosition, dir);
	if (abs(heightOff) > 150)
		return false;

	if (length(playerPos - position) < 200)
	{
		if (PlayerInFront())
			return true;
	}

	return false;
}

bool Crawler::TryAttack()
{
	if (ShouldAttack())
	{
		action = ATTACK;
		frame = 0;
		return true;
	}

	return false;
}

bool Crawler::ShouldDash()
{
	if (action != CRAWL && action != ROLL && action != DECIDE )
		return false;

	V2d playerPos = sess->GetPlayerPos(0);

	V2d dir;
	if (clockwise)
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise)
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double heightOff = cross(playerPos - mover->physBody.globalPosition, dir);
	if (abs(heightOff) > 150 )
		return false;

	if (length(playerPos - position) < 320)
	{
		if (PlayerInFront())
			return true;
	}
	return false;
}

bool Crawler::PlayerInFront()
{
	V2d playerPos = sess->GetPlayerPos(0);

	V2d dir;
	if (clockwise )
	{
		dir = normalize(mover->ground->v1 - mover->ground->v0);
	}
	else if (!clockwise )
	{
		dir = normalize(mover->ground->v0 - mover->ground->v1);
	}
	double alongDist = dot(playerPos - mover->physBody.globalPosition, dir);
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

	double max = 20.0;
	double am = mover->groundSpeed + amount;
	if (am > max)
		am = max;
	if (am < -max)
		am = -max;
	//cout << "speed: " << am << endl;
	mover->SetSpeed( am );
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
		length(sess->GetPlayerPos(0) - mover->physBody.globalPosition) < 400);
}

void Crawler::AttemptRunAwayBoost()
{
	if (action != ROLL && action != CRAWL && action != DASH && action != ATTACK)
		return;

	if (IsPlayerChasingMe())
	{
		Accelerate(.05);
	}
}