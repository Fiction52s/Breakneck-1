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

	double height = 192;
	ts = owner->GetTileset("Enemies/shroom_192x192.png", 192, 192);
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
	hitboxInfo->damage = 3*60;
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
	cutObject->SetSubRectFront(29);
	cutObject->SetSubRectBack(30);
	//cutObject->SetFlipHoriz(false);
	cutObject->rotateAngle = sprite.getRotation();
	//cutObject->SetCutRootPos( Vector2f( position ) );

	UpdateSprite();
}

void Shroom::ResetEnemy()
{
	jelly->Reset();
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
		if (length(owner->GetPlayer(0)->position - position) < 60)
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
	DrawSpriteIfExists(target, sprite);
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
	orig = position;
	action = RISING;
	initHealth = 40;
	health = initHealth;

	shootLimit = 40;
	hitLimit = 1;

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

	

	comboObj = new ComboObject(this);


	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;

	comboObj->enemyHitBody = new CollisionBody(2);

	comboObj->enemyHitBody->AddCollisionBox(0, hitBox);

	CollisionBox exBox;
	exBox.type = CollisionBox::Hit;
	exBox.isCircle = true;
	exBox.globalAngle = 0;
	exBox.offset.x = 0;
	exBox.offset.y = 0;
	exBox.rw = 128;
	exBox.rh = 128;

	comboObj->enemyHitBody->AddCollisionBox(1, exBox);

	comboObj->enemyHitboxFrame = 0;

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
	actionLength[SHOT] = 30;
	actionLength[EXPLODING] = 20;

	animFactor[WAIT] = 1;
	animFactor[APPEARING] = 2;
	animFactor[RISING] = 2;
	animFactor[DROOPING] = 2;
	animFactor[DISSIPATING] = 2;
	animFactor[SHOT] = 1;
	animFactor[EXPLODING] = 1;

	currentCycle = 0;
	cycleLimit = 3;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	UpdateSprite();
}

//HitboxInfo * ShroomJelly::IsHit(Actor *player)
//{
//	return NULL;
//}

void ShroomJelly::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		owner->GetPlayer(0)->ConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		action = SHOT;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;
		double speed = 10;

		comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

		switch (receivedHit->hDir)
		{
		case HitboxInfo::LEFT:
			dir = V2d(-1, 0);
			break;
		case HitboxInfo::RIGHT:
			dir = V2d(1, 0);
			break;
		case HitboxInfo::UP:
			dir = V2d(0, -1);
			break;
		case HitboxInfo::DOWN:
			dir = V2d(0, 1);
			break;
		case HitboxInfo::UPLEFT:
			dir = V2d(-1, -1);
			break;
		case HitboxInfo::UPRIGHT:
			dir = V2d(1, -1);
			break;
		case HitboxInfo::DOWNLEFT:
			dir = V2d(-1, 1);
			break;
		case HitboxInfo::DOWNRIGHT:
			dir = V2d(1, 1);
			break;
		default:
			assert(0);

		}
		dir = normalize(dir);

		velocity = dir * speed;

		owner->GetPlayer(0)->AddActiveComboObj(comboObj);
	}
}

void ShroomJelly::ComboHit()
{
	pauseFrames = 5;
	++currHits;
	if (currHits >= hitLimit)
	{
		action = EXPLODING;
		comboObj->enemyHitboxFrame = 1;
		velocity = V2d(0, 0);
		frame = 0;
	}
}

void ShroomJelly::ResetEnemy()
{
	comboObj->Reset();
	comboObj->enemyHitboxFrame = 0;
	position = orig;
	action = WAIT;
	currentCycle = 0;
	health = initHealth;
	frame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	slowMultiple = 1;
	velocity = V2d(0, 0);
	shootFrames = 0;
	currHits = 0;
}

void ShroomJelly::UpdateEnemyPhysics()
{
	V2d movement = velocity / numPhysSteps / (double)slowMultiple;
	position += movement;
}

void ShroomJelly::ProcessState()
{
	
	V2d playerPos = owner->GetPlayer(0)->position;
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
				velocity = V2d(0, 0);
			}
			break;
		case EXPLODING:
		case DISSIPATING:
			numHealth = 0;
			dead = true;
			owner->GetPlayer(0)->RemoveActiveComboObj(comboObj);
			break;
		case SHOT:
			action = EXPLODING;
			comboObj->enemyHitboxFrame = 1;
			velocity = V2d(0, 0);
			break;
		}
	}

	
	if (action == EXPLODING)
	{
		if (frame == 3)
		{
			
		}
	}
	else if (action == SHOT)
	{

	}
	else
	{
		if (action != DISSIPATING && action != APPEARING && action != WAIT )
		{
			if (abs(playerPos.x - position.x) < 10)
			{
				velocity.x = 0;
			}
			else if (playerPos.x > position.x)
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
}

void ShroomJelly::EnemyDraw(sf::RenderTarget *target)
{
	if (action == WAIT)
	{
		return;
	}

	DrawSpriteIfExists(target, sprite);
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
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect( 0 ));
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

	comboObj->enemyHitBody->GetCollisionBoxes(comboObj->enemyHitboxFrame)->front().globalPosition = position;
}

