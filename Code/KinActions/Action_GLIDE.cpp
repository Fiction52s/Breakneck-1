#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GLIDE_Start()
{
}

void Actor::GLIDE_End()
{
	frame = 0;
}

void Actor::GLIDE_Change()
{
	GlideAction();
}

void Actor::GLIDE_Update()
{
	if (currInput.LUp())
	{
		if (glideTurnFactor < 0)
		{
			glideTurnFactor = 0;
		}
		glideTurnFactor += glideTurnAccel;
		if (glideTurnFactor > maxGlideTurnFactor)
		{
			glideTurnFactor = maxGlideTurnFactor;
		}
		//RotateCCW(springVel, glideTurnFactor);
	}
	else if (currInput.LDown())
	{
		if (glideTurnFactor > 0)
		{
			glideTurnFactor = 0;
		}
		glideTurnFactor -= glideTurnAccel;
		if (glideTurnFactor < -maxGlideTurnFactor)
		{
			glideTurnFactor = -maxGlideTurnFactor;
		}
		//RotateCCW(springVel, glideTurnFactor);
		//grav = AddGravity(V2d(0, 0));
	}
	else
	{
		glideTurnFactor = 0;
	}


	if (facingRight)
	{
		RotateCCW(velocity, glideTurnFactor);
	}
	else
	{
		RotateCCW(velocity, -glideTurnFactor);
	}
}

void Actor::GLIDE_UpdateSprite()
{

	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	V2d sVel = velocity;//springVel + springExtra;
	if (facingRight)
	{
		double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(a);
	}
	else
	{
		double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(-a + 180);
	}

	if (scorpOn)
		SetAerialScorpSprite();
}