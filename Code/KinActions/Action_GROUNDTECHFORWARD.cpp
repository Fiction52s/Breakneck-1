#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::GROUNDTECHFORWARD_Start()
{
	if (currInput.LLeft())
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
		groundSpeed = 0;
	}
}

void Actor::GROUNDTECHFORWARD_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::GROUNDTECHFORWARD_Change()
{

}

void Actor::GROUNDTECHFORWARD_Update()
{
}

void Actor::GROUNDTECHFORWARD_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame/3, r);


	SetGroundedSpriteTransform();
}

void Actor::GROUNDTECHFORWARD_TransitionToAction(int a)
{
}

void Actor::GROUNDTECHFORWARD_TimeIndFrameInc()
{

}

void Actor::GROUNDTECHFORWARD_TimeDepFrameInc()
{

}

int Actor::GROUNDTECHFORWARD_GetActionLength()
{
	return 12 * 3;
}

Tileset * Actor::GROUNDTECHFORWARD_GetTileset()
{
	return GetActionTileset("tech_forward_80x80.png");
}