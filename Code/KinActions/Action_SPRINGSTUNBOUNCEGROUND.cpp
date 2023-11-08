#include "Actor.h"
#include <iostream>
#include "Session.h"
#include "SoundTypes.h"

using namespace std;
using namespace sf;


void Actor::SPRINGSTUNBOUNCEGROUND_Start()
{

}

void Actor::SPRINGSTUNBOUNCEGROUND_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNBOUNCEGROUND_Change()
{
	if( frame == 4 )
	{
		springStunFrames = aimLauncherStunFrames;
		ActivateSound(PlayerSounds::S_BOUNCEJUMP);
		framesInAir = 0;
		SetAction(SPRINGSTUNAIM);
		oldBounceEdge = bounceEdge;
		oldBounceNorm = bounceNorm;
		frame = 0;

		V2d bouncePos = bounceEdge->GetPosition(edgeQuantity);
		V2d bn = bounceNorm;
		double angle = atan2(bn.x, -bn.y);
		bouncePos += bn * 80.0;
		ActivateEffect(PLAYERFX_BOUNCE_BOOST, Vector2f(bouncePos), RadiansToDegrees(angle), 30, 1, facingRight);

		if (facingRight && velocity.x < 0)
			facingRight = false;
		else if (!facingRight && velocity.x > 0)
			facingRight = true;

		framesSinceBounce = 0;
		bounceEdge = NULL;
	}
}

void Actor::SPRINGSTUNBOUNCEGROUND_Update()
{
	velocity.x = 0;
	velocity.y = 0;
	groundSpeed = 0;
}

void Actor::SPRINGSTUNBOUNCEGROUND_UpdateSprite()
{
	int bounceFrame = 0;
	V2d bn = bounceNorm;//bounceEdge->Normal();

	bool bounceFacingRight = facingRight;

	if (bn.y <= 0 && bn.y > -steepThresh)
	{
		bounceFrame = 2;
	}
	else if (bn.y >= 0 && -bn.y > -steepThresh)
	{
		bounceFrame = 2;
		//bounceFacingRight = (bn.x > 0);
	}
	else if (bn.y == 0)
	{
		bounceFrame = 2;
	}
	else if (bn.y < 0)
	{
		bounceFrame = 0;//8
	}
	else if (bn.y > 0)
	{
		bounceFrame = 4;
	}


	SetSpriteTexture(action);

	bool r = false;
	if (bn.y > 0)
	{
		r = true;
	}
	//bool r = (bounceFacingRight && !reversed ) || (!bounceFacingRight && reversed );
	SetSpriteTile(bounceFrame, facingRight, r);

	double angle = 0;
	if (!approxEquals(abs(offsetX), b.rw))
	{
		if (reversed)
			angle = PI;
	}
	else
	{
		angle = atan2(bn.x, -bn.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);

	//double a = GetVectorAngleCW(normalize(bounceEdge->Along())) * 180 / PI;

	V2d norm = bounceEdge->Normal();
	double cwAngle = GetVectorAngleCW(norm);
	double ccwAngle = GetVectorAngleCCW(norm) - PI / 2;

	double a;
	if (bounceFrame == 0 || bounceFrame == 4 )
	{
		if (facingRight)
		{
			a = cwAngle * 180 / PI;
			a -= 90;
			//a -= 90;
			//sprite->setRotation(a + 90);
		}
		else
		{
			a = cwAngle * 180 / PI;
			a -= 90;
		}
	}
	else if (bounceFrame == 2)
	{
		a = cwAngle * 180 / PI;
		if (!facingRight)
		{
			a += 180;
		}
	}

	sprite->setRotation(a);

	sprite->setPosition(position.x, position.y);
	

	scorpSet = true;
}

void Actor::SPRINGSTUNBOUNCEGROUND_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNBOUNCEGROUND_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNBOUNCEGROUND_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNBOUNCEGROUND_GetActionLength()
{
	return 60;
}

const char * Actor::SPRINGSTUNBOUNCEGROUND_GetTilesetName()
{
	return "bounce_96x96.png";
}