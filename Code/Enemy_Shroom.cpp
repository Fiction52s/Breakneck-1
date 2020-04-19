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

Shroom::Shroom(bool p_hasMonitor, Edge *g, double q, int p_level)
	:Enemy(EnemyType::EN_SHROOM, p_hasMonitor, 1), ground(g), edgeQuantity(q)
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

	action = LATENT;

	double height = 192;
	ts = sess->GetTileset("Enemies/shroom_192x192.png", 192, 192);
	ts_aura = sess->GetTileset("Enemies/shroom_aura_192x192.png", 192, 192);
	sprite.setTexture(*ts->texture);
	auraSprite.setTexture(*ts_aura->texture);

	V2d gPoint = g->GetPosition(edgeQuantity);

	receivedHit = NULL;

	hitSound = sess->GetSound("Enemies/shroom_spark");

	gn = g->Normal();
	angle = atan2(gn.x, -gn.y);

	position = gPoint + gn * ( 40.0 * scale );

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setScale(scale, scale);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(gPoint.x, gPoint.y);
	sprite.setRotation(angle / PI * 180);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3*60;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(32, position);
	BasicCircleHitBodySetup(32, position );

	hitBody.hitboxInfo = hitboxInfo;

	

	frame = 0;

	jelly = new ShroomJelly( position, level);
	jelly->Reset();

	spawnRect = sf::Rect<double>(gPoint.x - 64, gPoint.y - 64, 64 * 2, 64 * 2);

	actionLength[LATENT] = 18;
	actionLength[HITTING] = 11;

	animFactor[LATENT] = 2;
	animFactor[HITTING] = 2;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(29);
	cutObject->SetSubRectBack(30);
	cutObject->SetScale(scale);
	//cutObject->SetFlipHoriz(false);
	cutObject->rotateAngle = sprite.getRotation();
	//cutObject->SetCutRootPos( Vector2f( position ) );

	UpdateSprite();
}

Shroom::~Shroom()
{
	delete jelly;
}

void Shroom::ResetEnemy()
{
	jelly->Reset();
	action = LATENT;
	frame = 0;
	dead = false;
	receivedHit = NULL;
	slowCounter = 1;
	jellySpawnable = true;
	slowMultiple = 1;

	SetHitboxes(&hitBody, 0);
	SetHurtboxes(&hurtBody, 0);
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

	V2d playerPos = sess->GetPlayerPos(0);

	switch (action)
	{
	case LATENT:
		if (length(playerPos - position) < 60)
		{
			action = HITTING;
			sess->ActivateSoundAtPos( position, hitSound);
			frame = 0;
		}
		break;
	case HITTING:
		if (length(playerPos - position) > 120)
		{
			action = LATENT;
			frame = 0;
		}
		break;
	}
}

void Shroom::DirectKill()
{
	Enemy::DirectKill();
	jellySpawnable = false;
}


void Shroom::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(auraSprite);
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

	SyncSpriteInfo(auraSprite, sprite);

	/*if (hasMonitor && !suppressMonitor)
	{
		keySprite->setTextureRect(ts_key->GetSubRect(owner->keyFrame / 5));
		keySprite->setOrigin(keySprite->getLocalBounds().width / 2,
			keySprite->getLocalBounds().height / 2);
		keySprite->setPosition(position.x, position.y);
	}*/
}

void Shroom::HandleNoHealth()
{
	if( jellySpawnable )
		sess->AddEnemy(jelly);
}

void Shroom::CheckedMiniDraw(sf::RenderTarget *target,
	sf::FloatRect &rect)
{
	jelly->CheckedMiniDraw(target, rect);
	Enemy::CheckedMiniDraw(target, rect);
}

void Shroom::SetZoneSpritePosition()
{
	jelly->SetZoneSpritePosition();
	Enemy::SetZoneSpritePosition();
}

ShroomJelly::ShroomJelly(V2d &pos, int p_level )
	:Enemy(EnemyType::EN_SHROOMJELLY, 0, 1, false )
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

	position = pos;
	orig = position;
	action = RISING;
	shootLimit = 40;
	hitLimit = 1;

	double height = 160;
	ts = sess->GetTileset("Enemies/shroom_jelly_160x160.png", 160, 160);
	ts_aura = sess->GetTileset("Enemies/shroom_jelly_aura_160x160.png", 160, 160);
	sprite.setTexture(*ts->texture);
	auraSprite.setTexture(*ts_aura->texture);

	receivedHit = NULL;

	floatSound = sess->GetSound("Enemies/shroom_float");

	angle = 0;

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setScale(scale, scale);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(position.x, position.y);	

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = .5;
	hitboxInfo->drainY = .5;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 5;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(32, position);
	BasicCircleHitBodySetup(32, position);
	hitBody.hitboxInfo = hitboxInfo;

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

	comboObj->enemyHitBody.SetupNumFrames(2);
	comboObj->enemyHitBody.SetupNumBoxesOnFrame(0, 1);
	comboObj->enemyHitBody.SetupNumBoxesOnFrame(1, 1);
	//= new CollisionBody(2);

	comboObj->enemyHitBody.AddCollisionBox(0, hitBody.GetCollisionBoxes(0).front() );

	CollisionBox exBox;
	exBox.isCircle = true;
	exBox.globalAngle = 0;
	exBox.offset.x = 0;
	exBox.offset.y = 0;
	exBox.rw = 128 * scale;
	exBox.rh = 128 * scale;

	comboObj->enemyHitBody.AddCollisionBox(1, exBox);

	comboObj->enemyHitboxFrame = 0;

	frame = 0;

	float halfWidth = ts->tileWidth / 2;
	float halfHeight = ts->tileHeight / 2;
	spawnRect = sf::Rect<double>(position.x - halfWidth, position.y 
		- halfHeight, halfWidth * 2, halfHeight * 2);

	actionLength[WAIT] = 30;
	actionLength[APPEARING] = 4;
	actionLength[RISING] = 17 * 2;
	actionLength[DROOPING] = 30;
	actionLength[DISSIPATING] = 6;
	actionLength[SHOT] = 30;
	actionLength[EXPLODING] = 20;

	animFactor[WAIT] = 1;
	animFactor[APPEARING] = 2;
	animFactor[RISING] = 2;
	animFactor[DROOPING] = 2;
	animFactor[DISSIPATING] = 3;
	animFactor[SHOT] = 1;
	animFactor[EXPLODING] = 1;

	currentCycle = 0;
	cycleLimit = 3;

	UpdateSprite();
}

ShroomJelly::~ShroomJelly()
{
	//delete comboObj;
}

void ShroomJelly::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
		action = SHOT;
		frame = 0;
		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		V2d dir;
		double speed = 10;

		comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;
		dir = normalize(receivedHit->hDir);
		/*switch (receivedHit->hDir)
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

		}*/
		//dir = normalize(dir);

		velocity = dir * speed;

		sess->PlayerAddActiveComboObj(comboObj);
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
	
	V2d playerPos = sess->GetPlayerPos(0);
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case WAIT:
			action = APPEARING;
			SetHitboxes(&hitBody, 0);
			SetHurtboxes(&hurtBody, 0);
			break;
		case APPEARING:
			action = RISING;
			sess->ActivateSoundAtPos( position, floatSound);
			break;
		case RISING:
			action = DROOPING;
			break;
		case DROOPING:
			action = RISING;
			sess->ActivateSoundAtPos( position, floatSound);
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
			sess->PlayerRemoveActiveComboer(comboObj);
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

	target->draw(auraSprite);
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
			+ ( ( frame/ animFactor[RISING]) % 17 ) ) );
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
		sprite.setTextureRect(ts->GetSubRect(21
			+ frame / animFactor[DISSIPATING]) );
		break;
	case EXPLODING:
		sprite.setTextureRect(ts->GetSubRect(0));
		break;
	}

	SyncSpriteInfo(auraSprite, sprite);

	if (hasMonitor && !suppressMonitor)
	{
		keySprite.setTextureRect(ts_key->GetSubRect(sess->keyFrame / 5));
		keySprite.setOrigin(keySprite.getLocalBounds().width / 2,
			keySprite.getLocalBounds().height / 2);
		keySprite.setPosition(position.x, position.y);
	}
}

