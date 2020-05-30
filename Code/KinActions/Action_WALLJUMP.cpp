#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WALLJUMP_Start()
{
	if (currInput.rightShoulder && !prevInput.rightShoulder)
	{
		if (currInput.LUp())
		{
			wallJumpBufferedAttack = UAIR; //none
		}
		else if (currInput.LDown())
		{
			wallJumpBufferedAttack = DAIR;
		}
		else
		{
			wallJumpBufferedAttack = FAIR;
		}
	}
	else
	{
		wallJumpBufferedAttack = WALLJUMP;
	}
}

void Actor::WALLJUMP_End()
{
	SetActionExpr(JUMP);
	frame = 1;
	holdJump = false;
}

void Actor::WALLJUMP_Change()
{
	if ((frame == 1 || (frame == 0 && slowCounter > 1)) && wallJumpBufferedAttack != WALLJUMP)
	{
		SetAction(wallJumpBufferedAttack);
		wallJumpBufferedAttack = WALLJUMP;
		frame = 0;
		return;
	}

	BasicAirAction();
}

void Actor::WALLJUMP_Update()
{
	if (frame == 0)
	{
		ExecuteWallJump();
	}
	else if (frame >= wallJumpMovementLimit)
	{
		AirMovement();
	}
}

void Actor::WALLJUMP_UpdateSprite()
{
	if(frame == 0 && slowCounter == 1)
	{




		//cout << "ACTIVATING WALLJUMP" << endl;
	}

	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}