#include "Actor.h"
#include "Session.h"

using namespace sf;
using namespace std;

void Actor::INTROBOOST_Start()
{
}

void Actor::INTROBOOST_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::INTROBOOST_Change()
{
}

void Actor::INTROBOOST_Update()
{
}

void Actor::INTROBOOST_UpdateSprite()
{
	if (frame == 0 && slowCounter == 1)
	{
		ActivateEffect(DrawLayer::IN_FRONT, ts_exitAura, position, false, 0, 8, 2, true, 55);
		ActivateEffect(DrawLayer::IN_FRONT, sess->GetTileset("Kin/FX/enter_fx_320x320.png", 320, 320), position, false, 0, 24, 2, true);
		//owner->cam.SetManual(true);
	}
	else if (frame == 20)
	{
		//owner->cam.SetManual(false);
	}
	SetSpriteTexture(action);
	//SetSpriteTile((frame / 2) + 110, facingRight);
	SetSpriteTile((frame / 2), facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::INTROBOOST_TransitionToAction(int a)
{

}

void Actor::INTROBOOST_TimeIndFrameInc()
{

}

void Actor::INTROBOOST_TimeDepFrameInc()
{

}

int Actor::INTROBOOST_GetActionLength()
{
	return 18 * 2;//22 * 2;
}

const char * Actor::INTROBOOST_GetTilesetName()
{
	return INTRO_GetTilesetName();//EXITBOOST_GetTilesetName();
}