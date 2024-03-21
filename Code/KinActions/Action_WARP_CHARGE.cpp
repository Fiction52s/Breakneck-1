#include "Actor.h"
#include "Session.h"
//#include "VisualEffects.h"

using namespace sf;
using namespace std;

void Actor::WARP_CHARGE_Start()
{
	ground = NULL;
	bounceEdge = NULL;
	grindEdge = NULL;
	offsetX = 0;
	reversed = false;
	velocity = V2d(0, 0);
}

void Actor::WARP_CHARGE_End()
{
	SetAction(WARP);
	sess->usedWarp = true;
}

void Actor::WARP_CHARGE_Change()
{
	if (!currInput.BackButtonDown() )
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::WARP_CHARGE_Update()
{
}

void Actor::WARP_CHARGE_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	sprite->setRotation(0);
}

void Actor::WARP_CHARGE_TransitionToAction(int a)
{

}

void Actor::WARP_CHARGE_TimeIndFrameInc()
{

}

void Actor::WARP_CHARGE_TimeDepFrameInc()
{

}

int Actor::WARP_CHARGE_GetActionLength()
{
	return 60;
}

const char * Actor::WARP_CHARGE_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}