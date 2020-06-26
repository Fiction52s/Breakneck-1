#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WALLCLING_Start()
{
}

void Actor::WALLCLING_End()
{
	frame = 0;
}

void Actor::WALLCLING_Change()
{
	if (!currInput.LDown() && ((facingRight && currInput.LRight()) || (!facingRight && currInput.LLeft())))
	{
		SetAction(WALLJUMP);
		frame = 0;
	}
	else if (TryDoubleJump())
	{
		return;
	}
	else if (currInput.LDown())
	{
		SetAction(JUMP);
		frame = 1;
		holdJump = false;
	}
	else //if( currInput.rightShoulder && !prevInput.rightShoulder )
	{
		bool normalSwing = currInput.rightShoulder && !prevInput.rightShoulder;
		bool rightStickSwing = (currInput.RDown() && !prevInput.RDown())
			|| (currInput.RLeft() && !prevInput.RLeft())
			|| (currInput.RUp() && !prevInput.RUp())
			|| (currInput.RRight() && !prevInput.RRight());

		if (normalSwing || (rightStickSwing && (currInput.RLeft() || currInput.RRight())))
		{
			SetAction(WALLATTACK);
			frame = 0;
		}
		/*else if (!normalSwing && (rightStickSwing && (currInput.RUp() || currInput.RDown())))
		{
		AirAttack();
		}*/
	}
}

void Actor::WALLCLING_Update()
{
	if (velocity.y > clingSpeed)
	{
		//cout << "running wallcling" << endl;
		velocity.y = clingSpeed;
	}
	AirMovement();
}

void Actor::WALLCLING_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::WALLCLING_TransitionToAction(int a)
{

}

void Actor::WALLCLING_TimeIndFrameInc()
{

}

void Actor::WALLCLING_TimeDepFrameInc()
{

}

int Actor::WALLCLING_GetActionLength()
{
	return 1;
}

Tileset * Actor::WALLCLING_GetTileset()
{
	return GetActionTileset("wallcling_64x64.png");
}