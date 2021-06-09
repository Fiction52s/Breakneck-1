#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::FREEFLIGHTSTUN_Start()
{
}

void Actor::FREEFLIGHTSTUN_End()
{
	frame = 0;
}

void Actor::FREEFLIGHTSTUN_Change()
{
	if (springStunFrames == 0)
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::FREEFLIGHTSTUN_Update()
{
	double driftFactor = 2.0;//1.0;
	double maxAccelSpeed = 30;//15;

	if (currInput.LUp())
	{
		if (velocity.y > -maxAccelSpeed)
		{
			velocity.y -= driftFactor;
		}

	}
	if (currInput.LDown())
	{
		if (velocity.y < maxAccelSpeed)
		{
			velocity.y += driftFactor;
		}
	}
	if (currInput.LLeft())
	{
		if (velocity.x > -maxAccelSpeed)
		{
			velocity.x -= driftFactor;
		}

	}
	if (currInput.LRight())
	{
		if (velocity.x < maxAccelSpeed)
		{
			velocity.x += driftFactor;
		}
	}
}

void Actor::FREEFLIGHTSTUN_UpdateSprite()
{
	SetSpriteTexture(JUMP);

	SetSpriteTile(1, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::FREEFLIGHTSTUN_TransitionToAction(int a)
{

}

void Actor::FREEFLIGHTSTUN_TimeIndFrameInc()
{

}

void Actor::FREEFLIGHTSTUN_TimeDepFrameInc()
{

}

int Actor::FREEFLIGHTSTUN_GetActionLength()
{
	return 8;
}

Tileset * Actor::FREEFLIGHTSTUN_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}