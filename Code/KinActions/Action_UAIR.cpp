#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::UAIR_Start()
{
	SetActionSuperLevel();
	if (currActionSuperLevel == 2)
	{
		SetAction(TESTSUPER);
	}
}

void Actor::UAIR_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::UAIR_Change()
{
	BasicAirAttackAction();
}

void Actor::UAIR_Update()
{
	CheckHoldJump();

	SetCurrHitboxes(uairHitboxes[speedLevel], frame);

	if (frame == 0 && slowCounter == 1)
	{
		ActivateSound(S_UAIR);
		currAttackHit = false;
	}

	if (wallJumpFrameCounter >= wallJumpMovementLimit)
	{
		AirMovement();
	}
}

void Actor::UAIR_UpdateSprite()
{
	Tileset *curr_ts = ts_uairSword[speedLevel];
	int startFrame = 0;
	showSword = true;
	//showSword = frame / 3 >= startFrame && frame / 3 <= 5;

	if (showSword)
	{
		uairSword.setTexture(*curr_ts->texture);
	}

	SetSpriteTexture(action);

	SetSpriteTile(frame, facingRight);


	Vector2i offset(0, 0);
	//Vector2i offset( 8, -24 );

	if (showSword)
	{
		if (facingRight)
		{
			uairSword.setTextureRect(curr_ts->GetSubRect(frame - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame - startFrame);
			uairSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}

		uairSword.setOrigin(uairSword.getLocalBounds().width / 2, uairSword.getLocalBounds().height / 2);
		uairSword.setPosition(position.x + offset.x, position.y + offset.y);
		uairSword.setRotation(0);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::UAIR_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::UAIR_TimeIndFrameInc()
{

}

void Actor::UAIR_TimeDepFrameInc()
{

}

int Actor::UAIR_GetActionLength()
{
	return 16;
}

Tileset * Actor::UAIR_GetTileset()
{
	return GetActionTileset("uair_96x96.png");
}