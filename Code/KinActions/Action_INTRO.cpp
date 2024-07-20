#include "Actor.h"
#include "Session.h"

using namespace sf;
using namespace std;

void Actor::INTRO_Start()
{
}

void Actor::INTRO_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::INTRO_Change()
{
}

void Actor::INTRO_Update()
{
}

void Actor::INTRO_UpdateSprite()
{
	if (frame == 0 && slowCounter == 1)
	{
		ActivateEffect(PLAYERFX_ENTER, Vector2f(position), 0, 22, 2, true);
		//ActivateEffect(DrawLayer::IN_FRONT, sess->GetSizedTileset("Kin/FX/fx_enter_256x256.png"), position, false, 0, 22, 2, true);
	}
	/*else if (frame / 2 >= 5)
	{
		SetSpriteTexture(action);
		SetSpriteTile((frame - 5) / 2, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		sprite->setRotation(0);
	}*/

	SetSpriteTexture(action);
	SetSpriteTile(frame / 2, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::INTRO_TransitionToAction(int a)
{

}

void Actor::INTRO_TimeIndFrameInc()
{

}

void Actor::INTRO_TimeDepFrameInc()
{

}

int Actor::INTRO_GetActionLength()
{
	return 18 * 2;
}

const char * Actor::INTRO_GetTilesetName()
{
	return "entrance_64x64.png";
}