#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GRINDLUNGE_Start()
{
}

void Actor::GRINDLUNGE_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::GRINDLUNGE_Change()
{
	if (currInput.rightShoulder && !prevInput.rightShoulder)
	{
		SetAction(GRINDSLASH);
		frame = 0;
	}
	else if (!BasicAirAction() && !DashButtonHeld())
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::GRINDLUNGE_Update()
{
}

void Actor::GRINDLUNGE_UpdateSprite()
{
	SetSpriteTexture(GRINDLUNGE);

	SetSpriteTile(1, facingRight);



	double angle = atan2(lungeNormal.x, -lungeNormal.y);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setRotation(angle / PI * 180);
	sprite->setPosition(position.x, position.y);
	//float angle = atan2( 

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::GRINDLUNGE_TransitionToAction(int a)
{

}

void Actor::GRINDLUNGE_TimeIndFrameInc()
{

}

void Actor::GRINDLUNGE_TimeDepFrameInc()
{

}

int Actor::GRINDLUNGE_GetActionLength()
{
	return 20;
}

const char * Actor::GRINDLUNGE_GetTilesetName()
{
	return "airdash_80x80.png";
}