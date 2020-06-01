#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDHITSTUN_Start()
{
}

void Actor::GROUNDHITSTUN_End()
{
	if (stunBufferedAttack == Action::Count)
	{
		if (currInput.rightShoulder && !prevInput.rightShoulder)
		{
			stunBufferedAttack = STANDN;
		}
	}

	if (!stunBufferedJump && currInput.A && !prevInput.A)
	{
		stunBufferedJump = true;
	}

	if (!stunBufferedDash && currInput.B && !prevInput.B)
	{
		stunBufferedDash = true;
	}
}

void Actor::GROUNDHITSTUN_Change()
{
	
}

void Actor::GROUNDHITSTUN_Update()
{
	hitstunFrames--;
	int slowDown = 1;
	if (groundSpeed > 0)
	{
		groundSpeed -= slowDown;
		if (groundSpeed < 0)
		{
			groundSpeed = 0;
		}
	}
	else if (groundSpeed < 0)
	{
		groundSpeed += slowDown;
		if (groundSpeed > 0)
		{
			groundSpeed = 0;
		}
	}
}

void Actor::GROUNDHITSTUN_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(1, r);


	double angle = GroundedAngle();



	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
}

void Actor::GROUNDHITSTUN_TransitionToAction(int a)
{

}

void Actor::GROUNDHITSTUN_TimeIndFrameInc()
{

}

void Actor::GROUNDHITSTUN_TimeDepFrameInc()
{

}

int Actor::GROUNDHITSTUN_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDHITSTUN_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}