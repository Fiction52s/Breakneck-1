#include "Enemy.h"
#include "Enemy_GravityModifier.h"
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

GravityModifier::GravityModifier(GameSession *owner, Vector2i &pos, double p_gFactor, int p_duration )
	:Enemy(owner, EnemyType::EN_GRAVITYMODIFIER, false, 1, false), gravFactor(p_gFactor), duration( p_duration )
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

	//animationFactor = 10;

	//ts = owner->GetTileset( "GravityModifier.png", 80, 80 );
	ts = owner->GetTileset("Enemies/booster_512x512.png", 512, 512);
	ts_refresh = owner->GetTileset("Enemies/booster_on_256x256.png", 256, 256);
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
	hitBox.rw = 90;
	hitBox.rh = 90;


	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hit;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 90;
	hurtBox.rh = 90;
	//need hitbox info?


	hitBody->AddCollisionBox(0, hitBox);
	hurtBody->AddCollisionBox(0, hurtBox);

	dead = false;

	//UpdateHitboxes();

	spawnRect = sf::Rect<double>(position.x - 100, position.y - 100,
		200, 200);

	SetHitboxes(hitBody, 0);
	SetHitboxes(hurtBody, 0);

	actionLength[NEUTRAL] = 6;
	actionLength[MODIFY] = 8;
	actionLength[REFRESH] = 7;

	animFactor[NEUTRAL] = 3;
	animFactor[MODIFY] = 3;
	animFactor[REFRESH] = 5;

	ResetEnemy();
}

bool GravityModifier::Modify()
{
	if (action == NEUTRAL)
	{
		action = MODIFY;
		frame = 0;
		return true;
	}
	return false;
}

bool GravityModifier::IsModifiable()
{
	return action == NEUTRAL;
}

void GravityModifier::ResetEnemy()
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

void GravityModifier::ProcessState()
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
		case MODIFY:
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

void GravityModifier::UpdateSprite()
{
	int tile = 0;
	IntRect ir;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		ir = ts->GetSubRect(tile);
		break;
	case MODIFY:
		tile = frame / animFactor[MODIFY] + actionLength[NEUTRAL];
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

void GravityModifier::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(sprite);
}

void GravityModifier::DrawMinimap(sf::RenderTarget *target)
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

void GravityModifier::UpdateHitboxes()
{
	//hurtBody.globalPosition = position;
	//hurtBody.globalAngle = 0;
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;
}

