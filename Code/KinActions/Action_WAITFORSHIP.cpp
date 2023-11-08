#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WAITFORSHIP_Start()
{
}

void Actor::WAITFORSHIP_End()
{
	frame = 0;
}

void Actor::WAITFORSHIP_Change()
{
}

void Actor::WAITFORSHIP_Update()
{
}

void Actor::WAITFORSHIP_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	if (ground != NULL)
	{
		SetGroundedSpriteTransform();
		

		//cout << "angle: " << angle / PI * 180  << endl;
	}
}

void Actor::WAITFORSHIP_TransitionToAction(int a)
{

}

void Actor::WAITFORSHIP_TimeIndFrameInc()
{

}

void Actor::WAITFORSHIP_TimeDepFrameInc()
{

}

int Actor::WAITFORSHIP_GetActionLength()
{
	return 60 * 1;
}

const char * Actor::WAITFORSHIP_GetTilesetName()
{
	return "ship_jump_160x96.png";
}