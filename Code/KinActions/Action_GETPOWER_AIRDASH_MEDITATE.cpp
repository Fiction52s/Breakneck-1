#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GETPOWER_AIRDASH_MEDITATE_Start()
{
}

void Actor::GETPOWER_AIRDASH_MEDITATE_End()
{
	SetAction(GETPOWER_AIRDASH_FLIP);
	frame = 0;
}

void Actor::GETPOWER_AIRDASH_MEDITATE_Change()
{
}

void Actor::GETPOWER_AIRDASH_MEDITATE_Update()
{
}

void Actor::GETPOWER_AIRDASH_MEDITATE_UpdateSprite()
{
	SetSpriteTexture(action);

	int f = min(frame / 3, 2);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(f, r);

	//assert(ground != NULL);

	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
	sprite->setRotation(angle / PI * 180);
}

void Actor::GETPOWER_AIRDASH_MEDITATE_TransitionToAction(int a)
{

}

void Actor::GETPOWER_AIRDASH_MEDITATE_TimeIndFrameInc()
{

}

void Actor::GETPOWER_AIRDASH_MEDITATE_TimeDepFrameInc()
{

}

int Actor::GETPOWER_AIRDASH_MEDITATE_GetActionLength()
{
	return 300;
}

Tileset * Actor::GETPOWER_AIRDASH_MEDITATE_GetTileset()
{
	return GetActionTileset("w1_airdashget_128x128.png");
}