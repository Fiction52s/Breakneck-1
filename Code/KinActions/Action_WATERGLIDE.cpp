#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WATERGLIDE_Start()
{
}

void Actor::WATERGLIDE_End()
{
	frame = 0;
}

void Actor::WATERGLIDE_Change()
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
		return;
	}

	if (DashButtonPressed())
	{
		SetAction(WATERGLIDECHARGE);
		frame = 0;
	}

	/*if (!GlideAction())
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
	}*/
}

void Actor::WATERGLIDE_Update()
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

	//cout << "springVel: " << springVel.x << ", " << springVel.y << endl;
	velocity = springVel + springExtra;
}

void Actor::WATERGLIDE_UpdateSprite()
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

void Actor::WATERGLIDE_TransitionToAction(int a)
{

}

void Actor::WATERGLIDE_TimeIndFrameInc()
{

}

void Actor::WATERGLIDE_TimeDepFrameInc()
{

}

int Actor::WATERGLIDE_GetActionLength()
{
	return 8;
}

const char * Actor::WATERGLIDE_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}