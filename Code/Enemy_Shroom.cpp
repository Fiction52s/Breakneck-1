#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Shroom.h"

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

Shroom::Shroom(GameSession *owner, bool p_hasMonitor, Edge *g, double q)
	:Enemy(owner, EnemyType::EN_SHROOM, p_hasMonitor, 1), ground(g), edgeQuantity(q)
{
	

	action = LATENT;
	initHealth = 40;
	health = initHealth;

	double height = 160;
	ts = owner->GetTileset("Enemies/shroom_160x160.png", 160, 160);
	sprite.setTexture(*ts->texture);

	V2d gPoint = g->GetPoint(edgeQuantity);

	receivedHit = NULL;

	gn = g->Normal();
	angle = atan2(gn.x, -gn.y);

	position = gPoint + gn * ( 40.0 );

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(gPoint.x, gPoint.y);
	sprite.setRotation(angle / PI * 180);

	hurtBody = new CollisionBody(1); //this wille ventually match animation
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

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

	frame = 0;

	jelly = new ShroomJelly(owner, position);
	jelly->Reset();

	spawnRect = sf::Rect<double>(gPoint.x - 64, gPoint.y - 64, 64 * 2, 64 * 2);

	actionLength[LATENT] = 18;
	actionLength[HITTING] = 11;

	animFactor[LATENT] = 2;
	animFactor[HITTING] = 2;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(30);
	cutObject->SetSubRectBack(29);
	cutObject->SetFlipHoriz(true);
	cutObject->rotateAngle = sprite.getRotation();
	cutObject->SetCutRootPos( Vector2f( position ) );

	UpdateSprite();
}

void Shroom::ResetEnemy()
{
	action = LATENT;
	health = initHealth;
	frame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
}

void Shroom::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case LATENT:
			frame = 0;
			break;
		case HITTING:
			frame = 0;
			break;
		}
	}

	switch (action)
	{
	case LATENT:
		if (length(owner->GetPlayer(0)->position - position) < 100)
		{
			action = HITTING;
			frame = 0;
		}
		break;
	case HITTING:
		if (length(owner->GetPlayer(0)->position - position) > 120)
		{
			action = LATENT;
			frame = 0;
		}
		break;
	}
}

void Shroom::EnemyDraw(sf::RenderTarget *target)
{
	cout << "shroomdraw" << endl;
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

void Shroom::UpdateSprite()
{
	switch (action)
	{
	case LATENT:
		sprite.setTextureRect(ts->GetSubRect((frame / animFactor[LATENT])));
		sprite.setPosition(position.x, position.y);
		break;
	case HITTING:
		sprite.setTextureRect(ts->GetSubRect( actionLength[LATENT] + frame / animFactor[HITTING]));
		sprite.setPosition(position.x, position.y);
		break;
	}

	if (hasMonitor && !suppressMonitor)
	{
		keySprite->setTextureRect(ts_key->GetSubRect(owner->keyFrame / 5));
		keySprite->setOrigin(keySprite->getLocalBounds().width / 2,
			keySprite->getLocalBounds().height / 2);
		keySprite->setPosition(position.x, position.y);
	}
}

void Shroom::HandleNoHealth()
{
	owner->AddEnemy(jelly);
}


void Shroom::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position - gn * 10.0;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position - gn * 10.0;
	hitBox.globalAngle = 0;
}

ShroomJelly::ShroomJelly(GameSession *owner, V2d &pos )
	:Enemy(owner, EnemyType::EN_SHROOMJELLY, 0, 1, false )
{
	position = pos;
	action = RISING;
	initHealth = 40;
	health = initHealth;

	double height = 160;
	ts = owner->GetTileset("Enemies/shroom_jelly_160x160.png", 160, 160);
	sprite.setTexture(*ts->texture);


	receivedHit = NULL;

	
	angle = 0;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(position.x, position.y);	

	hurtBody = new CollisionBody(1); //this wille ventually match animation
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

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

	frame = 0;

	float halfWidth = ts->tileWidth / 2;
	float halfHeight = ts->tileHeight / 2;
	spawnRect = sf::Rect<double>(position.x - halfWidth, position.y 
		- halfHeight, halfWidth * 2, halfHeight * 2);

	actionLength[WAIT] = 30;
	actionLength[APPEARING] = 3;
	actionLength[RISING] = 30;
	actionLength[DROOPING] = 30;
	actionLength[DISSIPATING] = 1;

	animFactor[WAIT] = 1;
	animFactor[APPEARING] = 2;
	animFactor[RISING] = 2;
	animFactor[DROOPING] = 2;
	animFactor[DISSIPATING] = 2;

	currentCycle = 0;
	cycleLimit = 3;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateSprite();
}

HitboxInfo * ShroomJelly::IsHit(Actor *player)
{
	return NULL;
}

void ShroomJelly::ResetEnemy()
{
	action = WAIT;
	currentCycle = 0;
	health = initHealth;
	frame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;
}

void ShroomJelly::UpdateEnemyPhysics()
{
	V2d movement = velocity / numPhysSteps / (double)slowMultiple;
	position += movement;
}

void ShroomJelly::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case WAIT:
			action = APPEARING;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			break;
		case APPEARING:
			action = RISING;
			break;
		case RISING:
			action = DROOPING;
			break;
		case DROOPING:
			action = RISING;
			currentCycle++;
			if (currentCycle == cycleLimit)
			{
				action = DISSIPATING;
			}
			break;
		case DISSIPATING:
			numHealth = 0;
			dead = true;
			break;
		}
	}

	V2d playerPos = owner->GetPlayer(0)->position;



	if (action != DISSIPATING && action != APPEARING && action != WAIT )
	{
		if (abs( playerPos.x - position.x ) < 10 )
		{
			velocity.x = 0;
		}
		else if( playerPos.x > position.x )
		{
			velocity.x = 5;
		}
		else
		{
			velocity.x = -5;
		}
	}
	
	if (action == RISING)
	{
		//double f = (double)frame / (double)animFactor[RISING]
		//	/ (double)actionLength[RISING];
		//risingBez.GetValue( f );
		velocity.y = -5;
		
		
	}
	else if (action == DROOPING)
	{
		velocity.y = 5;
	}

}

void ShroomJelly::EnemyDraw(sf::RenderTarget *target)
{
	if (action == WAIT)
	{
		return;
	}

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

void ShroomJelly::UpdateSprite()
{
	if (action == WAIT)
		return;
	sprite.setPosition(position.x, position.y);
	switch (action)
	{
	case APPEARING:
		sprite.setTextureRect(ts->GetSubRect((frame / animFactor[APPEARING])));
		break;
	case RISING:
		sprite.setTextureRect(ts->GetSubRect(actionLength[APPEARING] 
			+ ( ( frame/ animFactor[APPEARING]) % 15 ) ) );
		break;
	case DROOPING:
	{
		int f = frame / animFactor[DROOPING];
		if (f > 5)
			f = 5;
		sprite.setTextureRect(ts->GetSubRect(actionLength[APPEARING]
			+ f ) );
		break;
	}
	case DISSIPATING: 
		sprite.setTextureRect(ts->GetSubRect(18
			+ frame / animFactor[DISSIPATING]) );
		break;
	}

	if (hasMonitor && !suppressMonitor)
	{
		keySprite->setTextureRect(ts_key->GetSubRect(owner->keyFrame / 5));
		keySprite->setOrigin(keySprite->getLocalBounds().width / 2,
			keySprite->getLocalBounds().height / 2);
		keySprite->setPosition(position.x, position.y);
	}
}

void ShroomJelly::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;
}

