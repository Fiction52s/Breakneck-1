#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DAIR_Start()
{
	SetActionSuperLevel();

	ActivateSound(S_DAIR);
	ResetAttackHit();
}

void Actor::DAIR_End()
{
	SetAction(JUMP);
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
		TryThrowSwordProjectileBasic();
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
		swordSprite.setTexture(*curr_ts->texture);
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
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame - startFrame);
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}
		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height / 2);
		swordSprite.setPosition(position.x + offset.x, position.y + offset.y);
		swordSprite.setRotation(0);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::DAIR_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::DAIR_TimeIndFrameInc()
{

}

void Actor::DAIR_TimeDepFrameInc()
{

}

int Actor::DAIR_GetActionLength()
{
	return 16;
}

Tileset * Actor::DAIR_GetTileset()
{
	//return GetActionTileset("dair_80x80.png");
	return GetActionTileset("Paletted/dair_r_80x80.png");
}