#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDPARRYLOW_Start()
{
	/*if (oldAction != AIRBLOCK)
	{
		framesBlocking = 0;
	}*/
}

void Actor::GROUNDPARRYLOW_End()
{
	frame = 0;
}

void Actor::GROUNDPARRYLOW_Change()
{
	if (!PowerButtonHeld() && blockstunFrames == 0)
	{
		SetAction(STAND);
		frame = 0;
	}
}

void Actor::GROUNDPARRYLOW_Update()
{
	//double gAngle = GroundedAngle();
	////double fac = gravity * 2.0 / 3;
	//if (gAngle != 0 && gAngle != PI)
	//{
	//	if (reversed)
	//	{
	//		double accel = dot(V2d(0, slideGravFactor * GetGravity()), normalize(ground->v1 - ground->v0)) / slowMultiple;
	//		groundSpeed += accel;

	//	}
	//	else
	//	{
	//		double accel = dot(V2d(0, slideGravFactor * GetGravity()), normalize(ground->v1 - ground->v0)) / slowMultiple;
	//		groundSpeed += accel;
	//		//cout << "accel slide: \n" << accel;
	//	}
	//}
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
}

void Actor::GROUNDPARRYLOW_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();
}

void Actor::GROUNDPARRYLOW_TransitionToAction(int a)
{
}

void Actor::GROUNDPARRYLOW_TimeIndFrameInc()
{

}

void Actor::GROUNDPARRYLOW_TimeDepFrameInc()
{

}

int Actor::GROUNDPARRYLOW_GetActionLength()
{
	return 1;
}

const char * Actor::GROUNDPARRYLOW_GetTilesetName()
{
	return "parry_low_64x64.png";
}