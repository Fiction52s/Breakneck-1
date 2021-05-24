#include "Actor.h"
#include "VisualEffects.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNGLIDE_Start()
{
}

void Actor::SPRINGSTUNGLIDE_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNGLIDE_Change()
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

void Actor::SPRINGSTUNGLIDE_Update()
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
		RotateCCW(springVel, glideTurnFactor);
	}
	else
	{
		RotateCCW(springVel, -glideTurnFactor);
	}

	//springExtra = AddGravity(springExtra);
	//if (springVel.y > 0)
	if (false)
	{
		double gFac = 1.0;
		V2d nsv = normalize(springVel);
		springVel = nsv * (length(springVel) + gFac * nsv.y);//AddGravity(springVel);
	}


	//springExtra = tempVel - springVel;
	velocity = springVel + springExtra;

	float framesIn = springStunFramesStart - springStunFrames;
	float doneFactor = framesIn / springStunFramesStart;
	float scaleFactor = 1.f - doneFactor;//.5 + (1.f - doneFactor ) * .5;

	EffectInstance ef;
	Transform ti = Transform::Identity;
	ti.scale(scaleFactor, scaleFactor);

	V2d velDir = normalize(velocity);
	double angD = GetVectorAngleCW(velDir) / PI * 180.0;
	//cout << "angle: " << angD << endl;

	ti.rotate(angD);
	ef.SetParams(Vector2f( position ), ti, 7, 4, 0);
	//ef.SetVelocityParams( Vector2f( 0, )
	glideEffectPool->ActivateEffect(&ef);
}

void Actor::SPRINGSTUNGLIDE_UpdateSprite()
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

void Actor::SPRINGSTUNGLIDE_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNGLIDE_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNGLIDE_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNGLIDE_GetActionLength()
{
	return 8;
}

Tileset * Actor::SPRINGSTUNGLIDE_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}