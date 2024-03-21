#include "Actor.h"
#include "VisualEffects.h"

using namespace sf;
using namespace std;

void Actor::WATERGLIDECHARGE_Start()
{
}

void Actor::WATERGLIDECHARGE_End()
{
	frame = 0;
}

void Actor::WATERGLIDECHARGE_Change()
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

	if (!currInput.DashButtonDown())
	{
		SetAction(WATERGLIDE);
		frame = 0;

		//velocity = (springVel + springExtra) * factor;
	}
	
}

void Actor::WATERGLIDECHARGE_Update()
{
	V2d targetDir = currInput.GetLeft8Dir();

	double turnFactor;

	double len = length(springVel);
	/*double minSpeed = 25;

	if (len < minSpeed)
	{
		springVel = normalize(springVel) * minSpeed;
		len = minSpeed;
	}*/

	springVel += targetDir * len * .03;//.2;
	springVel = normalize(springVel) * len;

	double accel = .2;//.3;//.5;
	double decel = .1;
	double accelLimit = 40;

	if (len < accelLimit)
	{
		len += accel;
		if (len > accelLimit)
		{
			len = accelLimit;
		}
	}
	

	springVel = normalize(springVel) * len;

	V2d v = springVel + springExtra;
	double ang = GetVectorAngleCW(v);// / PI * 180.0;
	EffectInstance params;
	Transform t;

	//apparently you must rotate before scaling to get accurate results!
	t.rotate(ang / PI * 180.0);
	t.scale(1, 1);
	params.SetParams(Vector2f(position), t, 5, 4, 1);
	ActivateEffect(PLAYERFX_DASH_BOOST, &params);

	velocity = springVel + springExtra;

	//if (currInput.LUp())
	//{
	//	if (glideTurnFactor < 0)
	//	{
	//		glideTurnFactor = 0;
	//	}
	//	glideTurnFactor += glideTurnAccel;
	//	if (glideTurnFactor > maxGlideTurnFactor)
	//	{
	//		glideTurnFactor = maxGlideTurnFactor;
	//	}
	//}
	//else if (currInput.LDown())
	//{
	//	if (glideTurnFactor > 0)
	//	{
	//		glideTurnFactor = 0;
	//	}
	//	glideTurnFactor -= glideTurnAccel;
	//	if (glideTurnFactor < -maxGlideTurnFactor)
	//	{
	//		glideTurnFactor = -maxGlideTurnFactor;
	//	}
	//}
	//else
	//{
	//	glideTurnFactor = 0;
	//}

	//if (facingRight)
	//{
	//	RotateCCW(springVel, glideTurnFactor);
	//}
	//else
	//{
	//	RotateCCW(springVel, -glideTurnFactor);
	//}

	//double startSlowFactor = 1.0;
	//double minSlowSpeed = 10.0;
	////double minSlowFactor = .5;
	//double factor = startSlowFactor - .02 * frame;

	//double len = length(springVel + springExtra) * factor;
	//if (len < minSlowSpeed)
	//{
	//	velocity = normalize(springVel + springExtra) * minSlowSpeed;
	//}
	//else
	//{
	//	velocity = (springVel + springExtra) * factor;
	//}
	//cout << "springVel: " << springVel.x << ", " << springVel.y << endl;
	//velocity = (springVel + springExtra) * factor;
}

void Actor::WATERGLIDECHARGE_UpdateSprite()
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

void Actor::WATERGLIDECHARGE_TransitionToAction(int a)
{

}

void Actor::WATERGLIDECHARGE_TimeIndFrameInc()
{

}

void Actor::WATERGLIDECHARGE_TimeDepFrameInc()
{

}

int Actor::WATERGLIDECHARGE_GetActionLength()
{
	return -1; //infinite frames so I can count how long I'm in it
}

const char * Actor::WATERGLIDECHARGE_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}