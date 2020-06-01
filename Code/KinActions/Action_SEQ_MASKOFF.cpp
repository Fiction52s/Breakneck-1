#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::SEQ_MASKOFF_Start()
{
}

void Actor::SEQ_MASKOFF_End()
{
	action = SEQ_MEDITATE;
	frame = 0;
}

void Actor::SEQ_MASKOFF_Change()
{
}

void Actor::SEQ_MASKOFF_Update()
{
}

void Actor::SEQ_MASKOFF_UpdateSprite()
{
	SetSpriteTexture(action);
	bool r = (facingRight && !reversed) || (!facingRight && reversed);

	int f = frame / 3 + 11;
	SetSpriteTile(f, r);

	double angle = 0;//GroundedAngle()


	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - 16);
	sprite->setRotation(angle / PI * 180);

	V2d pp;
	if (ground != NULL)
		pp = ground->GetPosition(edgeQuantity);
	else
	{
		assert(0);
	}

	sprite->setPosition(pp.x, pp.y);
}

void Actor::SEQ_MASKOFF_TransitionToAction(int a)
{

}

void Actor::SEQ_MASKOFF_TimeIndFrameInc()
{

}

void Actor::SEQ_MASKOFF_TimeDepFrameInc()
{

}

int Actor::SEQ_MASKOFF_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_MASKOFF_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}