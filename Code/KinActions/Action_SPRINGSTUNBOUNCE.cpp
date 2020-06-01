#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNBOUNCE_Start()
{
}

void Actor::SPRINGSTUNBOUNCE_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNBOUNCE_Change()
{
	if (!GlideAction())
	{
		if (springStunFrames == 0)
		{
			SetAction(JUMP);
			frame = 1;
			if (velocity.x < 0)
			{
				facingRight = false;
			}
			else if (velocity.x > 0)
			{
				facingRight = true;
			}
		}
	}
	else
	{
		springStunFrames = 0;
	}
}

void Actor::SPRINGSTUNBOUNCE_Update()
{
	double bounceTurnFactor = .012;
	if (currInput.LUp())
	{
		glideTurnFactor = bounceTurnFactor;
		/*if (bounceTurnFactor < 0)
		{
		bounceTurnFactor = 0;
		}
		glideTurnFactor += glideTurnAccel;
		if (glideTurnFactor > maxGlideTurnFactor)
		{
		glideTurnFactor = maxGlideTurnFactor;
		}*/
		//RotateCCW(springVel, glideTurnFactor);
	}
	else if (currInput.LDown())
	{
		/*if (glideTurnFactor > 0)
		{
		glideTurnFactor = 0;
		}
		glideTurnFactor -= glideTurnAccel;
		if (glideTurnFactor < -maxGlideTurnFactor)
		{
		glideTurnFactor = -maxGlideTurnFactor;
		}*/
		glideTurnFactor = -bounceTurnFactor;
		//RotateCCW(springVel, glideTurnFactor);
		//grav = AddGravity(V2d(0, 0));
	}
	else
	{
		glideTurnFactor = 0;
	}


	if (facingRight)
	{
		RotateCCW(springVel, glideTurnFactor);
	}
	else
	{
		RotateCCW(springVel, -glideTurnFactor);
	}

	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUNBOUNCE_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	V2d sVel = springVel + springExtra;
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

void Actor::SPRINGSTUNBOUNCE_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNBOUNCE_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNBOUNCE_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNBOUNCE_GetActionLength()
{
	return 1;
}

Tileset * Actor::SPRINGSTUNBOUNCE_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}