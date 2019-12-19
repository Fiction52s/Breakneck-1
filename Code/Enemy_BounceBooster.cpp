#include "Enemy.h"
#include "Enemy_BounceBooster.h"
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

BounceBooster::BounceBooster(GameSession *owner, Vector2i &pos, bool p_upOnly, int p_level)
	:Enemy(owner, EnemyType::EN_BOUNCEBOOSTER, false, 1, false)
{
	level = p_level;
	strength = 20;
	upOnly = p_upOnly;

	switch (level)
	{
	case 1:
		scale = 1.0;
		if (upOnly)
		{
			strength = 30;
			sprite.setColor(Color::Red);
		}
		else
		{
			strength = 40;
		}
		
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		strength = 40;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		strength = 40;
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

	//ts = owner->GetTileset( "BounceBooster.png", 80, 80 );
	ts = owner->GetTileset("Enemies/Booster_512x512.png", 512, 512);
	ts_refresh = owner->GetTileset("Enemies/Booster_on_256x256.png", 256, 256);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);

	SetupBodies(1, 1);
	AddBasicHurtCircle(90);
	AddBasicHitCircle(90);


	dead = false;

	//UpdateHitboxes();

	spawnRect = sf::Rect<double>(position.x - 100, position.y - 100,
		200, 200);

	SetHitboxes(hitBody, 0);
	SetHitboxes(hurtBody, 0);

	actionLength[NEUTRAL] = 6;
	actionLength[BOOST] = 8;
	actionLength[REFRESH] = 7;

	animFactor[NEUTRAL] = 3;
	animFactor[BOOST] = 2;
	animFactor[REFRESH] = 2;

	ResetEnemy();
}

bool BounceBooster::Boost()
{
	if (action == NEUTRAL)
	{
		action = BOOST;
		frame = 0;
		return true;
	}
	return false;
}

bool BounceBooster::IsBoostable()
{
	return action == NEUTRAL;
}

void BounceBooster::ResetEnemy()
{
	action = NEUTRAL;
	dead = false;

	frame = 0;
	receivedHit = NULL;

	SetHitboxes(hitBody, 0);
	UpdateHitboxes();

	sprite.setTexture(*ts->texture);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(position.x, position.y);

	UpdateSprite();
}

void BounceBooster::ProcessState()
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
		case BOOST:
		{
			action = REFRESH;
			frame = 0;
			sprite.setTexture(*ts_refresh->texture);
			break;
		}
		case REFRESH:
		{
			action = NEUTRAL;
			frame = 0;
			sprite.setTexture(*ts->texture);
			break;
		}
		}
	}
}

void BounceBooster::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case BOOST:
		tile = frame / animFactor[BOOST] + actionLength[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case REFRESH:
		tile = frame / animFactor[REFRESH];
		ir = ts_refresh->GetSubRect(tile);
		break;
	}

	sprite.setTextureRect(ir);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(position.x, position.y);
}

void BounceBooster::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void BounceBooster::DrawMinimap(sf::RenderTarget *target)
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

