#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::INSPECT_END_Start()
{
}

void Actor::INSPECT_END_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::INSPECT_END_Change()
{
}

void Actor::INSPECT_END_Update()
{
	
}

void Actor::INSPECT_END_UpdateSprite()
{
	SetSpriteTexture(SLIDE);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::INSPECT_END_TransitionToAction(int a)
{

}

void Actor::INSPECT_END_TimeIndFrameInc()
{

}

void Actor::INSPECT_END_TimeDepFrameInc()
{

}

int Actor::INSPECT_END_GetActionLength()
{
	return 20;//STAND_GetActionLength();
}

const char * Actor::INSPECT_END_GetTilesetName()
{
	return NULL;
}