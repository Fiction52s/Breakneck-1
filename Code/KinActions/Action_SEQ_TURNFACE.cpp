#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_TURNFACE_Start()
{
}

void Actor::SEQ_TURNFACE_End()
{
	frame = 0;
}

void Actor::SEQ_TURNFACE_Change()
{
}

void Actor::SEQ_TURNFACE_Update()
{
}

void Actor::SEQ_TURNFACE_UpdateSprite()
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

void Actor::SEQ_TURNFACE_TransitionToAction(int a)
{

}

void Actor::SEQ_TURNFACE_TimeIndFrameInc()
{

}

void Actor::SEQ_TURNFACE_TimeDepFrameInc()
{

}

int Actor::SEQ_TURNFACE_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_TURNFACE_GetTileset()
{
	return GetActionTileset("ship_jump_160x96.png");
}