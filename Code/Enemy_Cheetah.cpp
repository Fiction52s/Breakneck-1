#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Cheetah.h"

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

Cheetah::Cheetah(GameSession *owner, bool hasMonitor, Edge *g, double q, int p_level)
	:Enemy(owner, EnemyType::EN_CHEETAH, hasMonitor, 2), facingRight(true),
	moveBezTest(.22, .85, .3, .91)
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

	maxGroundSpeed = 10;
	jumpStrength = 5;

	originalFacingRight = facingRight;
	actionLength[IDLE] = 10;
	actionLength[CHARGE] = 30;
	actionLength[BOOST] = 60;

	animFactor[IDLE] = 1;
	animFactor[CHARGE] = 1;
	animFactor[BOOST] = 1;

	//jumpStrength = p_jumpStrength;
	gravity = V2d(0, .6);

	action = IDLE;

	dead = false;

	maxFallSpeed = 25;

	attackFrame = -1;
	attackMult = 10;

	double width = 192;
	double height = 128;


	startGround = g;
	startQuant = q;
	frame = 0;

	mover = new GroundMover(owner, g, q, 32, true, this);
	mover->AddAirForce(V2d(0, .6));
	mover->SetSpeed(0);

	ts = owner->GetTileset("Enemies/Badger_192x128.png", width, height);
	ts_aura = owner->GetTileset("Enemies/Badger_aura_192x128.png", width, height);

	auraSprite.setTexture(*ts_aura->texture);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	auraSprite.setScale(scale, scale);

	V2d gPoint = g->GetPoint(q);


	position = mover->physBody.globalPosition;

	receivedHit = NULL;

	double size = max(width, height);
	spawnRect = sf::Rect<double>(gPoint.x - size, gPoint.y - size, size * 2, size * 2);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	SetupBodies(1, 1);
	AddBasicHurtCircle(32);
	AddBasicHitCircle(32);
	hitBody->hitboxInfo = hitboxInfo;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	bezLength = 60 * NUM_STEPS;

	ResetEnemy();

	//highResPhysics = true;
}

void Cheetah::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}

void Cheetah::ResetEnemy()
{
	facingRight = originalFacingRight;
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);

	position = mover->physBody.globalPosition;


	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	bezFrame = 0;
	attackFrame = -1;

	frame = 0;


	dead = false;

	action = IDLE;
	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Cheetah::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if (ground != NULL)
	{
		V2d knockbackDir(1, -1);
		knockbackDir = normalize(knockbackDir);
		if (mover->groundSpeed > 0)
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
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

//if (owner->GetPlayer(0)->position.x > position.x)
//{
//	//cout << "facing right" << endl;
//	facingRight = true;
//}
//else
//{
//	//cout << "facing left" << endl;
//	facingRight = false;
//}

void Cheetah::ActionEnded()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case CHARGE:
			action = BOOST;
			break;
		case BOOST:
			action = IDLE;
			frame = 0;
			break;
		}
	}
}

void Cheetah::Jump(double strengthx, double strengthy)
{
	V2d jumpVec = V2d(strengthx, -strengthy);
	mover->Jump(jumpVec);
}

void Cheetah::ProcessState()
{
	//cout << "vel: " << mover->velocity.x << ", " << mover->velocity.y << endl;
	//cout << "action: " << (int)action << endl;
	//testLaunch->UpdatePrePhysics();
	//Actor *player = owner->GetPlayer(0);

	V2d playerPos = owner->GetPlayerPos(0);

	if (dead)
		return;

	//cout << "vel: " << mover->velocity.x << ", " << mover->velocity.y << endl;

	double xDiff = playerPos.x - position.x;
	double dist = length(playerPos - position);
	ActionEnded();

	switch (action)
	{
	case IDLE:		
		if (dist < 500)
		{
			action = CHARGE;
			frame = 0;
		}
		break;
	case CHARGE:
		break;
	case BOOST:
		if (frame == 0)
		{
			if (xDiff >= 0)
			{
				mover->SetSpeed(100);
				facingRight = true;
			}
			else
			{
				mover->SetSpeed(-100);
				facingRight = false;
			}
		}
		break;
	}

	/*if (mover->ground == NULL)
	{
		double airAccel = .5;
		if (facingRight)
		{
			mover->velocity.x += airAccel;
			if (mover->velocity.x > 10)
			{
				mover->velocity.x = 10;
			}

		}
		else
		{
			mover->velocity.x -= airAccel;
			if (mover->velocity.x < -10)
				mover->velocity.x = -10;

		}
	}*/

	switch (action)
	{
	case IDLE:
		break;
	case CHARGE:

		break;
	case BOOST:
		if (mover->groundSpeed > 0)
		{
			if (xDiff < -300)
			{
				mover->SetSpeed(0);
				facingRight = false;
				action = IDLE;
				frame = 0;
			}
		}
		else if (mover->groundSpeed < 0)
		{
			if (xDiff > 300)
			{
				mover->SetSpeed(0);
				facingRight = true;
				action = IDLE;
				frame = 0;
			}
		}
		break;
	}
}

void Cheetah::UpdateEnemyPhysics()
{
	mover->Move(slowMultiple, numPhysSteps);

	if (mover->ground == NULL)
	{
		if (mover->velocity.y > maxFallSpeed)
		{
			mover->velocity.y = maxFallSpeed;
		}
	}

	position = mover->physBody.globalPosition;
}

void Cheetah::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(auraSprite);
	DrawSpriteIfExists(target, sprite);
}


void Cheetah::UpdateSprite()
{
	double angle = 0;

	V2d p = mover->physBody.globalPosition;
	V2d vel = mover->velocity;

	double groundSpeed = mover->groundSpeed;

	float extraVert = 0;

	if (mover->ground != NULL)
	{
		V2d gPoint = mover->ground->GetPoint(mover->edgeQuantity);
		V2d gn = mover->ground->Normal();

		if (!mover->roll)
		{
			angle = atan2(gn.x, -gn.y);

		}
		else
		{
			if (facingRight)
			{
				V2d vec = normalize(position - mover->ground->v1);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
			}
			else
			{
				V2d vec = normalize(position - mover->ground->v0);
				angle = atan2(vec.y, vec.x);
				angle += PI / 2.0;

				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
			}
		}

		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(gPoint.x, gPoint.y);
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setPosition(p.x, p.y);
		sprite.setRotation(0);
	}


	int airRange = 3;
	int fallRange = 15;
	sf::IntRect ir;
	int index = 0;
	switch (action)
	{
	case IDLE:
		index = 0;
		break;
	case CHARGE:
		index = 1;
		break;
	case BOOST:
		index = 2;
		break;
	}

	ir = ts->GetSubRect(index);

	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}
	sprite.setTextureRect(ir);

	SyncSpriteInfo(auraSprite, sprite);
}
//
//void Cheetah::HitTerrain( double &q )
//{
//	
//}

bool Cheetah::StartRoll()
{
	return false;
}

void Cheetah::FinishedRoll()
{

}

void Cheetah::HitOther()
{
	/*V2d v;
	if( facingRight && mover->groundSpeed > 0 )
	{
	v = V2d( 10, -10 );
	mover->Jump( v );
	}
	else if( !facingRight && mover->groundSpeed < 0 )
	{
	v = V2d( -10, -10 );
	mover->Jump( v );
	}*/
	//cout << "hit other!" << endl;
	//mover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Cheetah::ReachCliff()
{
	return;
	if (facingRight && mover->groundSpeed < 0
		|| !facingRight && mover->groundSpeed > 0)
	{
		mover->SetSpeed(0);
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v;
	if (facingRight)
	{
		v = V2d(10, -10);
	}
	else
	{
		v = V2d(-10, -10);
	}

	//action = LEDGEJUMP;
	frame = 0;

	Jump(v.x, v.y);
}

void Cheetah::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Cheetah::Land()
{
	return;
	//action = LAND;
	frame = 0;

	//cout << "land" << endl;
}