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

void Crawler::SetLevel(int p_level)
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
}

Crawler::Crawler(ActorParams *ap )
	:Enemy( EnemyType::EN_CRAWLER, ap ), facingClockwise( true ), groundSpeed( 5 )
{
	SetNumActions(A_Count);
	SetEditorActions(CRAWL, CRAWL, 0);

	SetLevel(ap->enemyLevel);

	maxFramesUntilBurrow = 200;
	dashAccel = .1;

	CreateSurfaceMover(startPosInfo, 32, this);

	deathSound = sess->GetSound("Enemies/crawler_death");
	ts = sess->GetSizedTileset("Enemies/crawler_160x160.png");
	ts_aura = sess->GetSizedTileset("Enemies/crawler_aura_160x160.png");

	auraSprite.setTexture(*ts_aura->texture);
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = surfaceMover->ground->GetPosition( startPosInfo.GetQuant() );
	sprite.setPosition( gPoint.x, gPoint.y );

	sprite.setScale(scale, scale);
	
	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(86);
	cutObject->SetSubRectBack(87);
	cutObject->SetScale(scale);
	
	V2d gNorm = surfaceMover->ground->Normal();
	double angle = atan2( gNorm.x, -gNorm.y );
	sprite.setRotation( angle / PI * 180.f );
	position = surfaceMover->physBody.globalPosition;

	actionLength[UNBURROW] = 20;
	actionLength[CRAWL] = 35;
	actionLength[ROLL] = 5;
	actionLength[DASH] = 7;
	actionLength[ATTACK] = 6;
	actionLength[BURROW] = 19;
	actionLength[UNDERGROUND] = 20;

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

	SetSpawnRect();

	ResetEnemy();
}

Crawler::~Crawler()
{
}

void Crawler::PlayDeathSound()
{
	Enemy::PlayDeathSound();
	sess->ActivateSoundAtPos( position, deathSound);
}

sf::FloatRect Crawler::GetAABB() 
{
	return sprite.getGlobalBounds();
}

void Crawler::ResetEnemy()
{
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	framesUntilBurrow = maxFramesUntilBurrow;
	
	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);

	position = surfaceMover->physBody.globalPosition;

	currDistTravelled = 0;

	frame = 0;

	V2d gn = surfaceMover->ground->Normal();
	dead = false;

	double angle = 0;
	angle = atan2( gn.x, -gn.y );


	action = UNBURROW;
	frame = 15 * 4;

	UpdateHitboxes();

	DecideDirection();

	UpdateSprite();
}

void Crawler::DecideDirection()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d gn = surfaceMover->ground->Normal();
	if (gn.y < 0)
	{
		if (playerPos.x < position.x)
		{
			facingClockwise = false;
		}
		else
		{
			facingClockwise = true;
		}
	}
	else if (gn.y > 0)
	{
		if (playerPos.x < position.x)
		{
			facingClockwise = true;
		}
		else
		{
			facingClockwise = false;
		}
	}
	else
	{
		if (gn.x < 0)
		{
			if (playerPos.y < position.y)
			{
				facingClockwise = false;
			}
			else
			{
				facingClockwise = true;
			}
		}
		else if (gn.x > 0)
		{
			if (playerPos.y > position.y)
			{
				facingClockwise = true;
			}
			else
			{
				facingClockwise = false;
			}
		}
	}
}

void Crawler::UpdateHitboxes()
{
	BasicUpdateHitboxes();
	
	if( surfaceMover->ground != NULL )
	{
		V2d gn = surfaceMover->ground->Normal();
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
		if( surfaceMover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = baseKB + max( abs( surfaceMover->groundSpeed ), maxExtraKB);
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = baseKB + max(abs(surfaceMover->groundSpeed), maxExtraKB);
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
	if (surfaceMover->ground->edgeType == Edge::OPEN_GATE)
	{
		DirectKill();
		return;
	}

	V2d en = surfaceMover->ground->Normal();

	if (action != BURROW && action != UNDERGROUND)
	{
		if (framesUntilBurrow == 0)
		{
			action = BURROW;
			frame = 0;
			surfaceMover->SetSpeed(0);
		}
	}

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case UNBURROW:
			action = DECIDE;
			frame = 0;
			if (facingClockwise)
			{
				surfaceMover->SetSpeed(groundSpeed);
			}
			else
			{
				surfaceMover->SetSpeed(-groundSpeed);
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
			HurtboxesOff();
			action = UNDERGROUND;
			frame = 0;
			surfaceMover->ground = startPosInfo.GetEdge();
			surfaceMover->edgeQuantity = startPosInfo.GetQuant();
			surfaceMover->roll = false;
			surfaceMover->UpdateGroundPos();
			break;
		case UNDERGROUND:
			DefaultHurtboxesOn();
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
			DefaultHitboxesOn();
		}
		break;
	case CRAWL:
		break;
	case DASH:
		break;
	case BURROW:
		if (frame == 4 * 12)
		{
			HurtboxesOff();
			HitboxesOff();
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
		V2d gn = surfaceMover->ground->Normal();
		double rollacc = .08;

		if ((surfaceMover->groundSpeed > 0 && gn.x < 0 && gn.y < 0) || (surfaceMover->groundSpeed < 0 && gn.x > 0 && gn.y < 0))
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
		surfaceMover->Move(slowMultiple, numPhysSteps );
		position = surfaceMover->physBody.globalPosition;
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
	V2d en = surfaceMover->ground->Normal();
	if (surfaceMover->groundSpeed > 0 && (en.x > 0 && en.y < 0)
		|| (surfaceMover->groundSpeed < 0 && en.x < 0 && en.y < 0))
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
	
	//cutObject->flipHoriz = !facingClockwise;
	cutObject->SetFlipHoriz(!facingClockwise);
	cutObject->rotateAngle = sprite.getRotation();
	
	//cutObject->SetCutRootPos(Vector2f(position.x, position.y));
	//action = DYING;
	//dead = true;
	//frame = 0;
	//int x = 5;
}

void Crawler::UpdateSprite()
{
	V2d gn = surfaceMover->ground->Normal();
	V2d gPoint = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);

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

	if (!facingClockwise)
	{
		sprite.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
	}
	else
	{
		sprite.setTextureRect(ir);
	}

	if (!surfaceMover->roll)
	{
		angle = atan2(gn.x, -gn.y);

		V2d pp = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);//ground->GetPosition( edgeQuantity );
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(pp.x, pp.y);
	}
	else
	{
		if (facingClockwise)
		{
			V2d vec = normalize(position - surfaceMover->ground->v1);
			angle = atan2(vec.y, vec.x);
			angle += PI / 2.0;

			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
			sprite.setRotation(angle / PI * 180);
			V2d pp = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);//ground->GetPosition( edgeQuantity );
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			V2d vec = normalize(position - surfaceMover->ground->v0);
			angle = atan2(vec.y, vec.x);
			angle += PI / 2.0;

			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
			sprite.setRotation(angle / PI * 180);
			V2d pp = surfaceMover->ground->GetPosition(surfaceMover->edgeQuantity);
			sprite.setPosition(pp.x, pp.y);
		}
	}

	if (editParams != NULL)
	{
		if (editParams->posInfo.IsAerial())
		{
			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			sprite.setPosition(editParams->GetFloatPos());
			sprite.setRotation(0);
			
		}
		else
		{
			
		}
		/*else
		{
			sprite.setPosition(editParams->GetFloatPos());
			sprite.setRotation(editParams->posInfo.GetGroundAngleDegrees());
		}*/
	}
	SyncSpriteInfo(auraSprite, sprite);
}

void Crawler::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
	//	surfaceMover->physBody.DebugDraw(target);
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
	if (facingClockwise)
	{
		dir = normalize(surfaceMover->ground->v1 - surfaceMover->ground->v0);
	}
	else if (!facingClockwise)
	{
		dir = normalize(surfaceMover->ground->v0 - surfaceMover->ground->v1);
	}
	double heightOff = cross(playerPos - surfaceMover->physBody.globalPosition, dir);
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
	if (facingClockwise)
	{
		dir = normalize(surfaceMover->ground->v1 - surfaceMover->ground->v0);
	}
	else if (!facingClockwise)
	{
		dir = normalize(surfaceMover->ground->v0 - surfaceMover->ground->v1);
	}
	double heightOff = cross(playerPos - surfaceMover->physBody.globalPosition, dir);
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
	if (facingClockwise )
	{
		dir = normalize(surfaceMover->ground->v1 - surfaceMover->ground->v0);
	}
	else if (!facingClockwise )
	{
		dir = normalize(surfaceMover->ground->v0 - surfaceMover->ground->v1);
	}
	double alongDist = dot(playerPos - surfaceMover->physBody.globalPosition, dir);
	if (alongDist > -60)
		return true;
	else
		return false;
}

void Crawler::Accelerate(double amount)
{
	if ( facingClockwise )
	{
		amount = abs(amount);
	}
	else
	{
		amount = -abs(amount);
	}

	double max = 20.0;
	double am = surfaceMover->groundSpeed + amount;
	if (am > max)
		am = max;
	if (am < -max)
		am = -max;
	//cout << "speed: " << am << endl;
	surfaceMover->SetSpeed( am );
}

void Crawler::SetForwardSpeed( double speed )
{
	double aSpeed = abs(speed);
	if (facingClockwise)
	{
		surfaceMover->SetSpeed(aSpeed);
	}
	else
	{
		surfaceMover->SetSpeed(-aSpeed);
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
		length(sess->GetPlayerPos(0) - surfaceMover->physBody.globalPosition) < 400);
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

void Crawler::ChildUpdateFromEditParams()
{
	if (!editParams->posInfo.IsAerial())
	{
		surfaceMover->ground = editParams->posInfo.GetEdge();
		surfaceMover->edgeQuantity = editParams->posInfo.GetQuant();

		startPosInfo = editParams->posInfo;
	}
}