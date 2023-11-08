#include "Actor.h"
#include "Enemy_SwingLauncher.h"

using namespace sf;
using namespace std;

void Actor::SWINGSTUN_Start()
{
}

void Actor::SWINGSTUN_End()
{
	frame = 0;
}

void Actor::SWINGSTUN_Change()
{
	if (!GlideAction())
	{
		/*if (springStunFrames == 0)
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
		}*/
	}
	else
	{
		springStunFrames = 0;
	}
}

void Actor::SWINGSTUN_Update()
{
	double rad = oldSwingLauncher->swingRadius;
	double speed = oldSwingLauncher->speed;

	

	V2d anchor = oldSwingLauncher->anchor;

	V2d dirToAnchor = normalize( position - anchor );

	//oldSwingLauncher->data.currAngle = GetVectorAngleCW(dirToAnchor);

	V2d future = position + normalize(springVel) * speed;//velocity;

	V2d diff = anchor - future;

	if (length(diff) > rad)
	{
		//future = anchor + normalize(diff) * rad;
		future += normalize(diff) * (length(diff) - rad);
		springVel = future - position;
	}

	V2d futureToAnchor = normalize(future - anchor);

	oldSwingLauncher->data.currAngle = GetVectorAngleCW(futureToAnchor);

	velocity = springVel; //* speed;
}

void Actor::SWINGSTUN_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	sprite->setRotation((oldSwingLauncher->data.currAngle - PI / 2) * 180 / PI);
}

void Actor::SWINGSTUN_TransitionToAction(int a)
{
	oldSwingLauncher->Recover();
}

void Actor::SWINGSTUN_TimeIndFrameInc()
{

}

void Actor::SWINGSTUN_TimeDepFrameInc()
{

}

int Actor::SWINGSTUN_GetActionLength()
{
	return 8;
}

const char * Actor::SWINGSTUN_GetTilesetName()
{
	return "swing_kin_64x64.png";
}