#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::GETPOWER_AIRDASH_FLIP_Start()
{
}

void Actor::GETPOWER_AIRDASH_FLIP_End()
{
	//SetStartUpgrade(POWER_AIRDASH, true);
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


	SetGroundedSpriteTransform();
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

const char * Actor::GETPOWER_AIRDASH_FLIP_GetTilesetName()
{
	return NULL;//"w1_airdashget_128x128.png";
}