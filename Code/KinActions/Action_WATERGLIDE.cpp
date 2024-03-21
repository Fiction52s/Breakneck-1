#include "Actor.h"
#include "VisualEffects.h"

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

	if (DashButtonHeld())
	{
		SetAction(WATERGLIDECHARGE);
		frame = 0;

		//double len = length(springVel);
		//double boostAmt = 10;
		//double accelLimit = 40;

		/*if (len + boostAmt < accelLimit )
		{
			springVel = normalize( springVel ) * len + boostAmt
		}*/
		


		return;
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
	V2d targetDir = currInput.GetLeft8Dir();

	double turnFactor;

	double len = length(springVel);
	springVel += targetDir * len * .15;//.2;//.2;
	springVel = normalize(springVel) * len;

	//double accel = .2;//.3;
	double decel = .15;//.2;
	//double accelLimit = 45;
	//if (DashButtonHeld())
	//{
	//	len += accel;
	//	if (len > accelLimit)
	//	{
	//		len = accelLimit;
	//	}
	//	
	//	springVel = normalize(springVel) * len;

	//	V2d v = springVel + springExtra;
	//	double ang = GetVectorAngleCW(v);// / PI * 180.0;
	//	EffectInstance params;
	//	Transform t;

	//	//apparently you must rotate before scaling to get accurate results!
	//	t.rotate(ang / PI * 180.0);
	//	t.scale(1, 1);
	//	params.SetParams(Vector2f(position), t, 5, 4, 1);
	//	ActivateEffect(PLAYERFX_DASH_BOOST, &params);
	//}
	//else
	{
		double dSpeed = GetDashSpeed();
		len -= decel;
		if (len < dSpeed)
		{
			len = dSpeed;
		}
		springVel = normalize(springVel) * len;
	}

	


	//double currTurnAccel = glideTurnAccel;
	//double currMaxTurn = maxGlideTurnFactor;

	//if (DashButtonPressed())
	//{
	//	//springVel = -springVel;

	//	if (springVel.x > 0 && !facingRight)
	//	{
	//		facingRight = true;
	//	}
	//	else if (springVel.x < 0 && facingRight)
	//	{
	//		facingRight = false;
	//	}
	//	//facingRight = !facingRight;
	//}
	//else if (JumpButtonPressed())
	//{
	//	springVel = -springVel;

	//	if (springVel.x > 0 && !facingRight)
	//	{
	//		facingRight = true;
	//	}
	//	else if (springVel.x < 0 && facingRight)
	//	{
	//		facingRight = false;
	//	}
	//}

	//if (currInput.LUp())
	////if (DashButtonHeld())
	//{
	//	if (glideTurnFactor < 0)
	//	{
	//		glideTurnFactor = 0;
	//	}
	//	glideTurnFactor += currTurnAccel;
	//	if (glideTurnFactor > currMaxTurn)
	//	{
	//		glideTurnFactor = currMaxTurn;
	//	}
	//}
	//else if (currInput.LDown())
	//{
	//	if (glideTurnFactor > 0)
	//	{
	//		glideTurnFactor = 0;
	//	}
	//	glideTurnFactor -= currTurnAccel;
	//	if (glideTurnFactor < -currMaxTurn)
	//	{
	//		glideTurnFactor = -currMaxTurn;
	//	}
	//}
	//else
	//{
	//	glideTurnFactor = 0;
	//}

	//V2d oldSpringVel = springVel;
	//if (facingRight)
	//{
	//	RotateCCW(springVel, glideTurnFactor);
	//}
	//else
	//{
	//	RotateCCW(springVel, -glideTurnFactor);
	//}




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