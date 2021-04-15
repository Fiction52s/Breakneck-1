#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_LOOKUP_Start()
{
}

void Actor::SEQ_LOOKUP_End()
{
	frame = 0;
}

void Actor::SEQ_LOOKUP_Change()
{
}

void Actor::SEQ_LOOKUP_Update()
{
}

void Actor::SEQ_LOOKUP_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();
}

void Actor::SEQ_LOOKUP_TransitionToAction(int a)
{

}

void Actor::SEQ_LOOKUP_TimeIndFrameInc()
{

}

void Actor::SEQ_LOOKUP_TimeDepFrameInc()
{

}

int Actor::SEQ_LOOKUP_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_LOOKUP_GetTileset()
{
	return GetActionTileset("cover_64x64.png");
}