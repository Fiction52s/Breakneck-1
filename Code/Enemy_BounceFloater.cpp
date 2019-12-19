#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Eye.h"
#include "KeyMarker.h"
#include "Enemy_BounceFloater.h"
#include "Actor.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )


BounceFloater::BounceFloater(GameSession *owner, Vector2i pos, int p_level)
	:Enemy(owner, EnemyType::EN_BOUNCEFLOATER, false, 2, false)
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

	action = S_FLOAT;
	position.x = pos.x;
	position.y = pos.y;

	origPos = position;

	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	frame = 0;

	//ts = owner->GetTileset( "BounceFloater.png", 80, 80 );
	ts = owner->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);
	sprite.setTextureRect(ts->GetSubRect(frame));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);
	sprite.setColor(Color::Red);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHurtCircle(48);
	AddBasicHitCircle(48);
	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	dead = false;

	actionLength[S_FLOAT] = 18;
	actionLength[S_BOUNCE] = 10;
	actionLength[S_RECOVER] = 10;

	animFactor[S_FLOAT] = 2;
	animFactor[S_BOUNCE] = 1;
	animFactor[S_RECOVER] = 1;

	ResetEnemy();
}

void BounceFloater::ResetEnemy()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);
	dead = false;
	action = S_FLOAT;
	frame = 0;
	receivedHit = NULL;
	position = origPos;
	UpdateHitboxes();

	UpdateSprite();
}

void BounceFloater::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0 && action == S_FLOAT )
	{
		Actor *player = owner->GetPlayer(0);

		SetHitboxes(NULL, 0);
		SetHurtboxes(NULL, 0);

		action = S_BOUNCE;
		frame = 0;

		V2d dir = receivedHit->hDir;

		if (player->ground != NULL)
		{
			player->groundSpeed = -player->groundSpeed;
		}
		else
		{
			double minX = 20;
			bool fr = player->facingRight;
			if (dir.x != 0 )//&& dir.y == 0)
			{
				double velx = player->velocity.x;
				if ( fr && velx < minX )
				{
					velx = minX;
				}
				else if (!fr && velx > -minX)
				{
					velx = -minX;
				}

				player->velocity.x = -velx;
					//= -player->velocity.x;
			}
			if (dir.y != 0 )//&& dir.x == 0)
			{
				double minUp = -20;
				double minDown = 40;
				double vely = player->velocity.y;
				if ( dir.y > 0 && vely < minDown)
				{
					vely = -minDown;
				}
				else if( dir.y < 0 && vely > minUp )
				{
					vely = -minUp;
				}

				//player->velocity.x = -velx;
				//= -player->velocity.x;



				player->velocity.y = vely;//-60;//player->velocity.y;
			}

		}

		player->ConfirmEnemyNoKill(this);
		ConfirmHitNoKill();
	}
}

void BounceFloater::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_FLOAT:
			break;
		case S_BOUNCE:
			action = S_RECOVER;
			break;
		case S_RECOVER:
			action = S_FLOAT;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			break;
		}
	}

	V2d playerPos = owner->GetPlayer(0)->position;
}

void BounceFloater::HandleNoHealth()
{

}

void BounceFloater::UpdateEnemyPhysics()
{
}

void BounceFloater::FrameIncrement()
{
}

void BounceFloater::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);

	int tile = 0;
	sprite.setTextureRect(ts->GetSubRect(tile));

	switch (action)
	{
	case S_FLOAT:
		break;
	}
}

void BounceFloater::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}