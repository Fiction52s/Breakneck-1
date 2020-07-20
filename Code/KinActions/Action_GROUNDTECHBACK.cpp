#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::GROUNDTECHBACK_Start()
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

void Actor::GROUNDTECHBACK_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::GROUNDTECHBACK_Change()
{

}

void Actor::GROUNDTECHBACK_Update()
{
}

void Actor::GROUNDTECHBACK_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame / 3, r);


	SetGroundedSpriteTransform();
}

void Actor::GROUNDTECHBACK_TransitionToAction(int a)
{
	stunBufferedJump = false;
	stunBufferedDash = false;
	stunBufferedAttack = Action::Count;
}

void Actor::GROUNDTECHBACK_TimeIndFrameInc()
{

}

void Actor::GROUNDTECHBACK_TimeDepFrameInc()
{

}

int Actor::GROUNDTECHBACK_GetActionLength()
{
	return 9 * 3;
}

Tileset * Actor::GROUNDTECHBACK_GetTileset()
{
	return GetActionTileset("tech_back_80x80.png");
}