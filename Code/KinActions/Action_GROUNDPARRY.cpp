#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDPARRY_Start()
{
}

void Actor::GROUNDPARRY_End()
{
	frame = 0;
}

void Actor::GROUNDPARRY_Change()
{
	if (!currInput.Y && blockstunFrames == 0)
	{
		SetAction(STAND);
		frame = 0;
	}
}

void Actor::GROUNDPARRY_Update()
{
}

void Actor::GROUNDPARRY_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();
}

void Actor::GROUNDPARRY_TransitionToAction(int a)
{
}

void Actor::GROUNDPARRY_TimeIndFrameInc()
{

}

void Actor::GROUNDPARRY_TimeDepFrameInc()
{

}

int Actor::GROUNDPARRY_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDPARRY_GetTileset()
{
	return GetActionTileset("parry_64x64.png");
}