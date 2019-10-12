#include "Enemy.h"
#include "Enemy_JugglerCatcher.h"
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

JugglerCatcher::JugglerCatcher(GameSession *owner, Vector2i &pos, int p_level)
	:Enemy(owner, EnemyType::EN_JUGGLERCATCHER, false, 1, false)
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

	action = NEUTRAL;
	frame = 0;
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;


	//spawnRect = sf::Rect<double>( pos.x - 16, pos.y - 16, 16 * 2, 16 * 2 );

	frame = 0;

	//animationFactor = 10;

	//ts = owner->GetTileset( "JugglerCatcher.png", 80, 80 );
	ts = owner->GetTileset("Enemies/jugglercatcher_128x128.png", 128, 128);
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
	actionLength[CATCH] = 8;
	actionLength[DEATH] = 7;

	animFactor[NEUTRAL] = 3;
	animFactor[CATCH] = 3;
	animFactor[DEATH] = 5;

	ResetEnemy();
}

bool JugglerCatcher::Catch()
{
	if (action == NEUTRAL)
	{
		action = CATCH;
		frame = 0;
		return true;
	}
	return false;
}

bool JugglerCatcher::CanCatch()
{
	return action == CATCH;
}

void JugglerCatcher::ResetEnemy()
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

void JugglerCatcher::ProcessState()
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
		case CATCH:
		{
			action = DEATH;
			frame = 0;
			break;
		}
		case DEATH:
		{
			dead = true;
			break;
		}
		}
	}
}

void JugglerCatcher::UpdateSprite()
{
	int tile = 0;
	IntRect ir;

	ir = ts->GetSubRect(0);
	/*switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case CATCH:
		tile = frame / animFactor[BOOST] + actionLength[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case DEATH:
		tile = frame / animFactor[REFRESH];
		ir = ts_refresh->GetSubRect(tile);
		break;
	}*/

	sprite.setTextureRect(ir);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(position.x, position.y);
}

void JugglerCatcher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void JugglerCatcher::DrawMinimap(sf::RenderTarget *target)
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

void JugglerCatcher::UpdateHitboxes()
{
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;
}

