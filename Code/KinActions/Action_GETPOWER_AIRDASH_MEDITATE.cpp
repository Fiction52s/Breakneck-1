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

	SetGroundedSpriteTransform();
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

const char * Actor::GETPOWER_AIRDASH_MEDITATE_GetTilesetName()
{
	return NULL;//"w1_airdashget_128x128.png";
}