#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BOUNCEGROUNDEDWALL_Start()
{

}

void Actor::BOUNCEGROUNDEDWALL_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::BOUNCEGROUNDEDWALL_Change()
{
	if (TryPressGrind()) return;


	if (JumpButtonPressed())
	{
		SetAction(JUMP);
		frame = 0;
		//runTappingSound.stop();
		return;
	}

	if (TryGroundAttack())
	{
		return;
	}
}

void Actor::BOUNCEGROUNDEDWALL_Update()
{
	if (frame == 0)
	{
		//cout << "storedBounceGround: " << groundSpeed << endl;
		//storedBounceGroundSpeed = groundSpeed;
		groundSpeed = 0;
	}
	else if (frame == 6)
	{

		groundSpeed = -storedBounceGroundSpeed / (double)slowMultiple;
		//cout << "set ground speed to: " << groundSpeed << endl;
	}
}

void Actor::BOUNCEGROUNDEDWALL_UpdateSprite()
{
	SetSpriteTexture(action);

	int tFrame = -1;
	if (frame < 6)
	{
		tFrame = 0;
	}
	else if (frame < 20)
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

	int yOffset = -75;
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
	}
	//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	sprite->setRotation(angle / PI * 180);
	V2d pp = ground->GetPosition(edgeQuantity);

	SetGroundedSpritePos(ground, angle);

	scorpSet = true;
}

void Actor::BOUNCEGROUNDEDWALL_TransitionToAction(int a)
{

}

void Actor::BOUNCEGROUNDEDWALL_TimeIndFrameInc()
{

}

void Actor::BOUNCEGROUNDEDWALL_TimeDepFrameInc()
{

}

int Actor::BOUNCEGROUNDEDWALL_GetActionLength()
{
	return 30;
}

Tileset * Actor::BOUNCEGROUNDEDWALL_GetTileset()
{
	return GetActionTileset("bounce_wall_224x224.png");
}