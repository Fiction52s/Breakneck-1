#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BRAKE_Start()
{

}

void Actor::BRAKE_End()
{
	frame = 0;
}

void Actor::BRAKE_Change()
{
	BasicGroundAction(currNormal);
}

void Actor::BRAKE_Update()
{
	BrakeMovement();
}

void Actor::BRAKE_UpdateSprite()
{
	SetSpriteTexture(BRAKE);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 15);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::BRAKE_TransitionToAction(int a)
{

}

void Actor::BRAKE_TimeIndFrameInc()
{

}

void Actor::BRAKE_TimeDepFrameInc()
{

}

int Actor::BRAKE_GetActionLength()
{
	return 1;
}

Tileset * Actor::BRAKE_GetTileset()
{
	return GetActionTileset("brake_64x64.png");
}