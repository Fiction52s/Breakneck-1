#include "Actor.h"
#include "Session.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_Start()
{
	springStunFrames = aimLauncherStunFrames;
	airBounceCounter++;
}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_End()
{
	SetAction(SPRINGSTUNAIRBOUNCE);

	V2d dir8 = currInput.GetLeft8Dir();

	if (dir8.x != 0 || dir8.y != 0)
	{
		springVel = dir8 * length(springVel);
	}
}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_Change()
{
	if (!JumpButtonHeld())
	{
		SetAction(SPRINGSTUNAIRBOUNCE);

		V2d dir8 = currInput.GetLeft8Dir();

		if (dir8.x != 0 || dir8.y != 0)
		{	
			springVel = dir8 * length(springVel);
			
		}
	}
}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_Update()
{
	velocity = V2d(0, 0);
}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNAIRBOUNCEPAUSE_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNAIRBOUNCEPAUSE_GetActionLength()
{
	return 10;
}

Tileset * Actor::SPRINGSTUNAIRBOUNCEPAUSE_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}