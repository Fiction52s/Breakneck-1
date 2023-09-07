#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BOOSTERBOUNCEGROUND_Start()
{
	//groundSpeed = //-storedBounceGroundSpeed / (double)slowMultiple;
}

void Actor::BOOSTERBOUNCEGROUND_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::BOOSTERBOUNCEGROUND_Change()
{
	if (CheckSetToAerialFromNormalWater()) return;

	if (TryGroundBlock()) return;


	if (TryFloorRailDropThrough()) return;

	if (TryPressGrind()) return;

	if (TryJumpSquat()) return;

	if (TryGroundAttack()) return;

	if (TryDash()) return;
}

void Actor::BOOSTERBOUNCEGROUND_Update()
{
}

void Actor::BOOSTERBOUNCEGROUND_UpdateSprite()
{
	SetSpriteTexture(action);

	int tFrame = -1;
	if (frame < 20)
	{
		tFrame = 1;
	}
	else
	{
		tFrame = 2;
	}

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(tFrame, r);

	double angle = 0;
	if (!approxEquals(abs(offsetX), b.rw))
	{
		if (reversed)
			angle = PI;
	}
	else
	{
		angle = atan2(currNormal.x, -currNormal.y);
	}

	//i have no idea what this code was doing or why
	/*int yOffset = -75;
	if (frame < 6)
	{
		if ((facingRight && !reversed) || (!facingRight && reversed))
		{
			sprite->setOrigin(sprite->getLocalBounds().width / 2 - 3, sprite->getLocalBounds().height + yOffset);
		}
		else
		{
			sprite->setOrigin(sprite->getLocalBounds().width / 2 + 3, sprite->getLocalBounds().height + yOffset);
		}

		angle = 0;
		if (reversed)
			angle = PI;
	}
	else
	{
		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height + yOffset);
	}*/
	int yOffset = -20;
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height + yOffset);// +yOffset);
	//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	sprite->setRotation(angle / PI * 180);
	V2d pp = ground->GetPosition(edgeQuantity);

	SetGroundedSpritePos(ground, angle);

	scorpSet = true;
}

void Actor::BOOSTERBOUNCEGROUND_TransitionToAction(int a)
{

}

void Actor::BOOSTERBOUNCEGROUND_TimeIndFrameInc()
{

}

void Actor::BOOSTERBOUNCEGROUND_TimeDepFrameInc()
{

}

int Actor::BOOSTERBOUNCEGROUND_GetActionLength()
{
	return 20;
}

Tileset * Actor::BOOSTERBOUNCEGROUND_GetTileset()
{
	return GetActionTileset("bounce_wall_96x64.png");
	//return GetActionTileset("brake_64x64.png");
}