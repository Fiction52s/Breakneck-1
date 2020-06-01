#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::SEQ_MEDITATE_MASKON_Start()
{
}

void Actor::SEQ_MEDITATE_MASKON_End()
{
	frame = 0;
}

void Actor::SEQ_MEDITATE_MASKON_Change()
{
}

void Actor::SEQ_MEDITATE_MASKON_Update()
{
}

void Actor::SEQ_MEDITATE_MASKON_UpdateSprite()
{
	SetSpriteTexture(action);
	bool r = (facingRight && !reversed) || (!facingRight && reversed);

	int f = 8;
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

void Actor::SEQ_MEDITATE_MASKON_TransitionToAction(int a)
{

}

void Actor::SEQ_MEDITATE_MASKON_TimeIndFrameInc()
{

}

void Actor::SEQ_MEDITATE_MASKON_TimeDepFrameInc()
{

}

int Actor::SEQ_MEDITATE_MASKON_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_MEDITATE_MASKON_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}