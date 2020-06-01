#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_WAIT_Start()
{
}

void Actor::SEQ_WAIT_End()
{
	frame = 0;
}

void Actor::SEQ_WAIT_Change()
{
}

void Actor::SEQ_WAIT_Update()
{
}

void Actor::SEQ_WAIT_UpdateSprite()
{
	sprite->setTexture(*(tileset[JUMP]->texture));
	{
		sf::IntRect ir;

		ir = tileset[JUMP]->GetSubRect(3);


		sprite->setRotation(0);

		if ((!facingRight && !reversed))
		{
			sprite->setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
		}
		else
		{
			sprite->setTextureRect(ir);
		}
	}
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
}

void Actor::SEQ_WAIT_TransitionToAction(int a)
{

}

void Actor::SEQ_WAIT_TimeIndFrameInc()
{

}

void Actor::SEQ_WAIT_TimeDepFrameInc()
{

}

int Actor::SEQ_WAIT_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_WAIT_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}