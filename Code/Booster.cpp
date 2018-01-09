#include "Enemy.h"
#include "Enemy_Booster.h"
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

Booster::Booster(GameSession *owner, Vector2i &pos, int p_strength)
	:Enemy(owner, EnemyType::EN_BOOSTER, false, 3), strength( p_strength )
{

	action = NEUTRAL;
	frame = 0;
	initHealth = 60;
	health = initHealth;

	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	initHealth = 20;
	health = initHealth;

	//spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	frame = 0;

	animationFactor = 10;

	//ts = owner->GetTileset( "Booster.png", 80, 80 );
	ts = owner->GetTileset("booster_64x64.png", 64, 64);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;

	//need hitbox info?

	
	hitBody->AddCollisionBox(0, hitBox);

	dead = false;

	UpdateHitboxes();

	spawnRect = sf::Rect<double>(position.x - 32, position.y - 32,
		64, 64);

	SetHitboxes(hitBody, 0);
}


void Booster::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	frame = 0;
	receivedHit = NULL;
	
	SetHitboxes(hitBody, 0);
	UpdateHitboxes();

	UpdateSprite();
}

void Booster::ProcessState()
{
	switch (action)
	{
	case NEUTRAL:
	{
		if (frame == 10)
		{
			frame = 0;
		}
		break;
	}
	case BOOST:
	{
		if (frame == 10)
		{
			action = REFRESH;
			frame = 0;
		}
		break;
	}
	case REFRESH:
	{
		if (frame == 60)
		{
			action = NEUTRAL;
			frame = 0;
		}
		break;
	}
	}
}

void Booster::UpdateSprite()
{
	IntRect ir = ts->GetSubRect(0);
	sprite.setTextureRect(ir);
}

void Booster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void Booster::DrawMinimap(sf::RenderTarget *target)
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

void Booster::UpdateHitboxes()
{
	//hurtBody.globalPosition = position;
	//hurtBody.globalAngle = 0;
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;
}

