#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include <cstdlib>
#include "Enemy_GravityFaller.h"

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



GravityFaller::GravityFaller(GameSession *p_owner, bool p_hasMonitor, Edge *g, double q, int fallerVariation)
	:Enemy(p_owner, EnemyType::EN_GRAVITYFALLER, p_hasMonitor, 2)
{
	maxFallSpeed = 25;
	actionLength[IDLE] = 10;
	actionLength[DOWNCHARGE] = 10;
	actionLength[FALLDOWN] = 10;
	actionLength[UPCHARGE] = 10;
	actionLength[FALLUP] = 10;
	actionLength[REVERSEUPTODOWN] = 10;
	actionLength[REVERSEDOWNTOUP] = 10;


	animFactor[IDLE] = 1;
	animFactor[DOWNCHARGE] = 1;
	animFactor[FALLDOWN] = 1;
	animFactor[UPCHARGE] = 1;
	animFactor[FALLUP] = 1;
	animFactor[REVERSEUPTODOWN] = 1;
	animFactor[REVERSEDOWNTOUP] = 1;

	double width = 128;
	double height = 128;
	ts = owner->GetTileset("Enemies/gravity_faller_128x128.png", width, height);

	
	mover = new SurfaceMover(p_owner, g, q, 30);
	mover->surfaceHandler = this;
	mover->SetSpeed(0);

	initHealth = 60;
	health = initHealth;

	//cout << "creating the boss crawler" << endl;
	action = IDLE;
	gravity = gravityFactor / 64.0;
	facingRight = false;
	receivedHit = NULL;


	dead = false;
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint(q);

	sprite.setTexture(*ts->texture);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setPosition(gPoint.x, gPoint.y);


	double size = max(width * 5, height * 5);
	spawnRect = sf::Rect<double>(gPoint.x - size / 2, gPoint.y - size / 2, size, size);

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = false;
	hurtBox.globalAngle = PI / 4.0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 40;
	hurtBox.rh = 40;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = false;
	hitBox.globalAngle = PI / 4.0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 40;
	hitBox.rh = 40;
	hitBody->AddCollisionBox(0, hitBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hitBody->hitboxInfo = hitboxInfo;

	startGround = g;
	startQuant = q;
	frame = 0;
	position = mover->physBody.globalPosition;//gPoint + ground->Normal() * physBody.rh; //16.0;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(6);
	cutObject->SetSubRectBack(5);

	ResetEnemy();

	chargeLength = 20;
}

void GravityFaller::ResetEnemy()
{
	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed(0);

	fallFrames = 0;
	position = mover->physBody.globalPosition;

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);
	dead = false;

	chargeFrame = 0;

	mover->ClearAirForces();

	//----

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateHitboxes();

	action = IDLE;
	frame = 0;

	facingRight = false;

	UpdateSprite();
	//groundSpeed = 0;
}

void GravityFaller::FrameIncrement()
{
	if (action == DOWNCHARGE || action == UPCHARGE)
	{
		chargeFrame++;
	}
	else if (action == FALLDOWN || action == FALLUP)
	{
		fallFrames++;
	}
}

void GravityFaller::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if (ground != NULL)
	{
		V2d gn = ground->Normal();

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
		//hitBody.globalAngle = angle;
		//hurtBody.globalAngle = angle;
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );

	hurtBody->GetCollisionBoxes(0)->front().globalPosition = position;
	hitBody->GetCollisionBoxes(0)->front().globalPosition = position;
}

void GravityFaller::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}
}

void GravityFaller::ProcessState()
{
	ActionEnded();

	Actor *player = owner->GetPlayer(0);

	if (action == IDLE)
	{
		if (length(player->position - position) < 500)
		{
			if (mover->ground->Normal().y < 0)
			{
				action = UPCHARGE;
				frame = 0;
				chargeFrame = 0;
			}
			else
			{
				action = DOWNCHARGE;
				frame = 0;
				chargeFrame = 0;
			}
		}
	}

	if (chargeFrame == chargeLength && ( action == DOWNCHARGE || action == UPCHARGE))
	{
		mover->ClearAirForces();
		if (action == DOWNCHARGE)
		{
			action = FALLDOWN;
			frame = 0;
			mover->Jump(V2d(0, 0));
			mover->AddAirForce(V2d(0, 1));
			fallFrames = 0;
		}
		else if (action == UPCHARGE)
		{
			action = FALLUP;
			frame = 0;
			mover->Jump(V2d(0, 0));
			mover->AddAirForce(V2d(0, -1));
			fallFrames = 0;
		}
	}
	else if ((action == FALLDOWN || action == FALLUP) && fallFrames == 40)
	{
		mover->ClearAirForces();
		if (action == FALLDOWN)
		{
			action = REVERSEDOWNTOUP;
			frame = 0;
			mover->AddAirForce(V2d(0, -1));
		}
		else if (action == FALLUP)
		{
			action = REVERSEUPTODOWN;
			frame = 0;
			mover->AddAirForce(V2d(0, 1));
		}
		
	}
	/*else if (action == REVERSEUPTODOWN && mover->velocity.y > 0)
	{
		action = FALLDOWN;
		frame = 0;
		fallFrames = 0;
	}
	else if (action == REVERSEDOWNTOUP && mover->velocity.y < 0)
	{
		action = FALLUP;
		frame = 0;
		fallFrames = 0;
	}*/

	//cout << "vel: " << mover->velocity.x << ", " << mover->velocity.y << endl;
}

void GravityFaller::UpdateEnemyPhysics()
{
	if (mover->ground != NULL)
	{
	}
	else
	{
		//double grav = gravity;
		////if (action == WALLCLING)
		////{
		////	grav = 0;//.1 * grav;
		////}
		//mover->velocity.y += grav / (numPhysSteps * slowMultiple);

		if (mover->velocity.y >= maxFallSpeed)
		{
			mover->velocity.y = maxFallSpeed;
		}
		else if (mover->velocity.y <= -maxFallSpeed)
		{
			mover->velocity.y = -maxFallSpeed;
		}
	}

	mover->Move(slowMultiple, numPhysSteps);

	position = mover->physBody.globalPosition;
}



void GravityFaller::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}

void GravityFaller::UpdateSprite()
{
	Edge *ground = mover->ground;
	double edgeQuantity = mover->edgeQuantity;
	V2d pp;

	if (ground != NULL)
	{
		pp = ground->GetPoint(edgeQuantity);
	}

	if (hasMonitor && !suppressMonitor)
	{
		keySprite->setTextureRect(ts_key->GetSubRect(owner->keyFrame / 2));
		keySprite->setOrigin(keySprite->getLocalBounds().width / 2,
			keySprite->getLocalBounds().height / 2);
		keySprite->setPosition(position.x, position.y);
	}

	sprite.setTexture(*ts->texture);

	int tIndex = 0;
	switch (action)
	{
	case IDLE:
		tIndex = 0;
		break;
	case DOWNCHARGE:
		tIndex = 3;
		break;
	case FALLDOWN:
	case REVERSEUPTODOWN:
		tIndex = 4;
		break;
	case UPCHARGE:
	case REVERSEDOWNTOUP:
		tIndex = 1;
		break;
	case FALLUP:
		tIndex = 2;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(tIndex));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(0);
	sprite.setPosition(position.x, position.y);




	/*if (!facingRight)
	{
		sf::IntRect r = sprite.getTextureRect();
		sprite.setTextureRect(sf::IntRect(r.left + r.width, r.top, -r.width, r.height));
	}*/
}

void GravityFaller::HitTerrainAerial(Edge * e, double quant)
{
	if (action == FALLDOWN || action == REVERSEDOWNTOUP )
	{
		action = UPCHARGE;
	}
	else if (action == FALLUP || action == REVERSEUPTODOWN )
	{
		action = DOWNCHARGE;
	}

	frame = 0;
	chargeFrame = 0;
}