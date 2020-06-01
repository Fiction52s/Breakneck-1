#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::GETPOWER_AIRDASH_FLIP_Start()
{
}

void Actor::GETPOWER_AIRDASH_FLIP_End()
{
	hasPowerAirDash = true;
	SetAction(STAND);
	frame = 0;
}

void Actor::GETPOWER_AIRDASH_FLIP_Change()
{
}

void Actor::GETPOWER_AIRDASH_FLIP_Update()
{
}

void Actor::GETPOWER_AIRDASH_FLIP_UpdateSprite()
{
	SetSpriteTexture(action);

	//int f = min( frame / 2, 11 );
	int f = frame / 2;

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(f, r);

	assert(ground != NULL);


	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - 16);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
	sprite->setRotation(angle / PI * 180);
}

void Actor::GETPOWER_AIRDASH_FLIP_TransitionToAction(int a)
{

}

void Actor::GETPOWER_AIRDASH_FLIP_TimeIndFrameInc()
{

}

void Actor::GETPOWER_AIRDASH_FLIP_TimeDepFrameInc()
{

}

int Actor::GETPOWER_AIRDASH_FLIP_GetActionLength()
{
	return 133 * 2;
}

Tileset * Actor::GETPOWER_AIRDASH_FLIP_GetTileset()
{
	return GetActionTileset("w1_airdashget_128x128.png");
}