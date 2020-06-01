#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::SEQ_KNEEL_Start()
{
}

void Actor::SEQ_KNEEL_End()
{
	frame = 0;
}

void Actor::SEQ_KNEEL_Change()
{
}

void Actor::SEQ_KNEEL_Update()
{
}

void Actor::SEQ_KNEEL_UpdateSprite()
{
	SetSpriteTexture(action);
	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

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

void Actor::SEQ_KNEEL_TransitionToAction(int a)
{

}

void Actor::SEQ_KNEEL_TimeIndFrameInc()
{

}

void Actor::SEQ_KNEEL_TimeDepFrameInc()
{

}

int Actor::SEQ_KNEEL_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_KNEEL_GetTileset()
{
	return GetActionTileset("kin_meditate_64x96.png");
}