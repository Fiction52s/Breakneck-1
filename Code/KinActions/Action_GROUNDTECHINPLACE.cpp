#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::GROUNDTECHINPLACE_Start()
{
	groundSpeed = 0;
	/*if (currInput.LLeft())
	{
		if (groundSpeed >= -10)
		{
			groundSpeed = -10;
		}
	}
	else if (currInput.LRight())
	{
		if (groundSpeed <= 10)
		{
			groundSpeed = 10;
		}
	}
	else
	{
		
	}*/
}

void Actor::GROUNDTECHINPLACE_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::GROUNDTECHINPLACE_Change()
{

}

void Actor::GROUNDTECHINPLACE_Update()
{
}

void Actor::GROUNDTECHINPLACE_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame / 2, r);


	SetGroundedSpriteTransform();
}

void Actor::GROUNDTECHINPLACE_TransitionToAction(int a)
{
}

void Actor::GROUNDTECHINPLACE_TimeIndFrameInc()
{

}

void Actor::GROUNDTECHINPLACE_TimeDepFrameInc()
{

}

int Actor::GROUNDTECHINPLACE_GetActionLength()
{
	return 12 * 2;
}

Tileset * Actor::GROUNDTECHINPLACE_GetTileset()
{
	return GetActionTileset("tech_stand_80x80.png");
}