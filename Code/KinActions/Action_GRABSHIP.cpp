#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GRABSHIP_Start()
{
}

void Actor::GRABSHIP_End()
{
}

void Actor::GRABSHIP_Change()
{
}

void Actor::GRABSHIP_Update()
{
}

void Actor::GRABSHIP_UpdateSprite()
{
	/*if (frame / 5 < 3)
	{
		
	}
	else
	{
		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
	}*/

	

	if (frame / 5 < 8)
	{
		SetSpriteTexture(action);

		//bool r = (facingRight && !reversed ) || (!facingRight && reversed );
		SetSpriteTile(1 + frame / 5, true);

		//sprite->setOrigin(sprite->getLocalBounds().width / 2,
		//	sprite->getLocalBounds().height - 20);
	}
	else
	{
		SetSpriteTexture(action);
		SetSpriteTile(8, true);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height - 48);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
	
}

void Actor::GRABSHIP_TransitionToAction(int a)
{

}

void Actor::GRABSHIP_TimeIndFrameInc()
{

}

void Actor::GRABSHIP_TimeDepFrameInc()
{

}

int Actor::GRABSHIP_GetActionLength()
{
	return 10 * 5 + 20;
}

const char * Actor::GRABSHIP_GetTilesetName()
{
	return "ship_jump_160x96.png";
}