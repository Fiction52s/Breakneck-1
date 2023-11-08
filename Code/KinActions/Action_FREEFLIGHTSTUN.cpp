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
	FreeFlightMovement();
	//double accelFactor = 1.0;//1.1;
	//double decelFactor = 2.0;//1.1;
	////double driftFactor = 1.1;//2.0;//1.0;
	//double maxAccelSpeed = 30;//15;

	//if (currInput.LUp())
	//{
	//	if (velocity.y > -maxAccelSpeed)
	//	{
	//		if (velocity.y > 0)
	//		{
	//			velocity.y -= decelFactor;
	//		}
	//		else
	//		{
	//			velocity.y -= accelFactor;
	//		}
	//	}

	//}
	//if (currInput.LDown())
	//{
	//	if (velocity.y < maxAccelSpeed)
	//	{
	//		if (velocity.y < 0)
	//		{
	//			velocity.y += decelFactor;
	//		}
	//		else
	//		{
	//			velocity.y += accelFactor;
	//		}
	//		
	//	}
	//}
	//if (currInput.LLeft())
	//{
	//	if (velocity.x > -maxAccelSpeed)
	//	{
	//		if (velocity.x > 0)
	//		{
	//			velocity.x -= decelFactor;
	//		}
	//		else
	//		{
	//			velocity.x -= accelFactor;
	//		}
	//	}

	//}
	//if (currInput.LRight())
	//{
	//	if (velocity.x < maxAccelSpeed)
	//	{
	//		if (velocity.x < 0)
	//		{
	//			velocity.x += decelFactor;
	//		}
	//		else
	//		{
	//			velocity.x += accelFactor;
	//		}
	//	}
	//}
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

const char * Actor::FREEFLIGHTSTUN_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}