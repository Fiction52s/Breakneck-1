#include "Actor.h"
#include "Enemy_InspectObject.h"

using namespace sf;
using namespace std;

void Actor::INSPECT_START_Start()
{
}

void Actor::INSPECT_START_End()
{
	frame = 0;
}

void Actor::INSPECT_START_Change()
{
}

void Actor::INSPECT_START_Update()
{
	if (frame == 20)
	{
		currInspectObject->ShowInspectable();
	}
}

void Actor::INSPECT_START_UpdateSprite()
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

void Actor::INSPECT_START_TransitionToAction(int a)
{

}

void Actor::INSPECT_START_TimeIndFrameInc()
{

}

void Actor::INSPECT_START_TimeDepFrameInc()
{

}

int Actor::INSPECT_START_GetActionLength()
{
	return 40;//STAND_GetActionLength();
}

const char * Actor::INSPECT_START_GetTilesetName()
{
	return NULL;
}