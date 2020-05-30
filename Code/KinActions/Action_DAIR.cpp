#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DAIR_Start()
{

}

void Actor::DAIR_End()
{
	SetActionExpr(JUMP);
	frame = 1;
}

void Actor::DAIR_Change()
{
	BasicAirAttackAction();
}

void Actor::DAIR_Update()
{
	CheckHoldJump();

	SetCurrHitboxes(dairHitboxes[speedLevel], frame);

	if (frame == 0 && slowCounter == 1)
	{

		ActivateSound(S_DAIR);
		if (speedLevel == 0)
		{

		}
		/*else if (speedLevel == 1)
		{
		ActivateSound(soundBuffers[S_DAIR_B]);
		}*/

		currAttackHit = false;
	}


	if (wallJumpFrameCounter >= wallJumpMovementLimit)
	{
		AirMovement();
	}
}

void Actor::DAIR_UpdateSprite()
{
	Tileset *curr_ts = ts_dairSword[speedLevel];
	int startFrame = 0;
	//showSword = frame / 2 >= startFrame && frame / 2 <= 9;
	showSword = true;

	if (showSword)
	{
		dairSword.setTexture(*curr_ts->texture);
	}

	Vector2i offsetArr[3];
	offsetArr[0] = Vector2i(0, 0);
	offsetArr[1] = Vector2i(0, 0);//Vector2i( 0, 48 );
	offsetArr[2] = Vector2i(0, 0);

	Vector2i offset = offsetArr[speedLevel];

	SetSpriteTexture(action);

	SetSpriteTile(frame, facingRight);

	if (showSword)
	{
		if (facingRight)
		{
			dairSword.setTextureRect(curr_ts->GetSubRect(frame - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame - startFrame);
			dairSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}
	}

	if (showSword)
	{
		dairSword.setOrigin(dairSword.getLocalBounds().width / 2, dairSword.getLocalBounds().height / 2);
		dairSword.setPosition(position.x + offset.x, position.y + offset.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}