#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Airdasher.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


Airdasher::Airdasher(GameSession *owner, bool p_hasMonitor, Vector2i pos)
	:Enemy(owner, EnemyType::EN_AIRDASHER, p_hasMonitor, 1, false)
{
	action = S_FLOAT;
	//receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;
	origPos = position;

	initHealth = 80;
	health = initHealth;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	frame = 0;

	//ts = owner->GetTileset( "Airdasher.png", 80, 80 );
	ts = owner->GetTileset("Enemies/blocker_w1_192x192.png", 192, 192);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(pos.x, pos.y);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 16;
	hurtBox.rh = 16;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 16;
	hitBox.rh = 16;
	hitBody = new CollisionBody(1);
	hitBody->AddCollisionBox(0, hitBox);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	//hitboxInfo->kbDir;

	targetNode = 1;
	forward = true;

	dead = false;

	
	facingRight = true;

	UpdateHitboxes();

	actionLength[S_FLOAT] = 18;
	actionLength[S_DASH] = 30;
	actionLength[S_RETURN] = 60;

	animFactor[S_FLOAT] = 2;
	animFactor[S_DASH] = 1;
	animFactor[S_RETURN] = 1;

	ResetEnemy();
}

void Airdasher::ResetEnemy()
{
	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	dead = false;
	action = S_FLOAT;
	frame = 0;
	position = origPos;
	receivedHit = NULL;

	UpdateHitboxes();

	UpdateSprite();
	health = initHealth;
}

void Airdasher::ProcessHit()
{
	Enemy::ProcessHit();

	//might add more later to return
}

void Airdasher::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_DASH:
			{
			action = S_RETURN;
				break;
			}
		case S_RETURN:
			action = S_FLOAT;
			break;
		}
	}
	V2d playerPos = owner->GetPlayer(0)->position;
}

void Airdasher::UpdateEnemyPhysics()
{
	V2d dest = origPos + playerDir * dashRadius;
	switch (action)
	{
	case S_DASH:
	{
		double a = (double)physStepIndex / 

		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		position += movementVec;
		break;
	}
	}
}

void Airdasher::FrameIncrement()
{
	/*if (action == CHARGEDFLAP)
	{
	++fireCounter;
	}*/
}

void Airdasher::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);
}

void Airdasher::EnemyDraw(sf::RenderTarget *target)
{
	RenderStates rs;
	rs.texture = ts->texture;
	if (hasMonitor && !suppressMonitor)
	{
		if (owner->pauseFrames < 2 || receivedHit == NULL)
		{
			rs.shader = keyShader;
			target->draw(sprite, keyShader);
		}
		else
		{
			rs.shader = hurtShader;
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
			rs.shader = hurtShader;
			target->draw(sprite, hurtShader);
		}
	}
}

void Airdasher::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if (owner->GetPlayer(0)->ground != NULL)
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->groundSpeed * (owner->GetPlayer(0)->ground->v1 - owner->GetPlayer(0)->ground->v0));
	}
	else
	{
		hitboxInfo->kbDir = normalize(-owner->GetPlayer(0)->velocity);
	}
}