#include "Enemy.h"
#include "Enemy_HealthFly.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>


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

HealthFly::HealthFly(GameSession *owner, Vector2i &pos, int p_level, int index)
	:Enemy(owner, EnemyType::EN_HEALTHFLY, false, 1, false), flyIndex( index )
{
	level = p_level;
	va = NULL;

	switch (level)
	{
	case 1:
		scale = 1.0;
		healAmount = 20;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		healAmount = 40;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		healAmount = 80;
		break;
	}

	action = NEUTRAL;
	frame = 0;
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;


	//spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	frame = 0;

	//animationFactor = 10;

	//ts = owner->GetTileset( "HealthFly.png", 80, 80 );
	ts = owner->GetTileset("Enemies/healthfly_64x64.png", 64, 64);
	//sprite.setTexture(*ts->texture);
	//sprite.setTextureRect(ts->GetSubRect(frame));
	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	//sprite.setScale(scale, scale);
	//sprite.setPosition(pos.x, pos.y);

	SetupBodies(1, 1);
	AddBasicHurtCircle(40);
	AddBasicHitCircle(40);


	dead = false;

	spawnRect = sf::Rect<double>(position.x - 100, position.y - 100,
		200, 200);

	SetHitboxes(hitBody, 0);
	SetHitboxes(hurtBody, 0);

	actionLength[NEUTRAL] = 5;
	actionLength[DEATH] = 8;

	animFactor[NEUTRAL] = 3;
	animFactor[DEATH] = 3;

	ResetEnemy();
}
//making it not heal when its dead!
void HealthFly::HandleQuery(QuadTreeCollider * qtc)
{
	if (!dead)
	{
		qtc->HandleEntrant(this);
	}
}

bool HealthFly::Collect()
{
	if (action == NEUTRAL)
	{
		action = DEATH;
		frame = 0;
		return true;
	}
	return false;
}

bool HealthFly::CanCollect()
{
	return true;
}

void HealthFly::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	frame = 0;
	receivedHit = NULL;

	SetHitboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	//UpdateHitboxes();

	//sprite.setTexture(*ts->texture);

	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	//sprite.setPosition(position.x, position.y);

	UpdateSprite();
}

void HealthFly::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case NEUTRAL:
		{
			frame = 0;
			break;
		}
		case DEATH:
		{
			numHealth = 0;
			ClearSprite();
			dead = true;
			break;
		}
		}
	}
}

void HealthFly::ClearSprite()
{
	ClearRect(va + flyIndex * 4);
}

void HealthFly::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		break;
	case DEATH:
		tile = 0;
		break;
	}

	ir = ts->GetSubRect(tile);

	if (va != NULL)
	{
		SetRectSubRect(va + flyIndex * 4, ir);
		SetRectCenter(va + flyIndex * 4, 64 * scale, 64 * scale, Vector2f(position.x, position.y));
	}
	
	//sprite.setTextureRect(ir);

	//sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	//sprite.setPosition(position.x, position.y);
}

void HealthFly::EnemyDraw(sf::RenderTarget *target)
{
	//target->draw(sprite);
}

void HealthFly::DrawMinimap(sf::RenderTarget *target)
{
	if (!dead)
	{
		CircleShape enemyCircle;
		enemyCircle.setFillColor(COLOR_BLUE);
		enemyCircle.setRadius(50);
		enemyCircle.setOrigin(enemyCircle.getLocalBounds().width / 2, enemyCircle.getLocalBounds().height / 2);
		enemyCircle.setPosition(position.x, position.y);
		target->draw(enemyCircle);
	}
}

void HealthFly::UpdateHitboxes()
{
}

