#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GRINDSLASH_Start()
{
}

void Actor::GRINDSLASH_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::GRINDSLASH_Change()
{
	BasicAirAttackAction();
}

void Actor::GRINDSLASH_Update()
{
	SetCurrHitboxes(uairHitboxes[0], frame);
}

void Actor::GRINDSLASH_UpdateSprite()
{
	Tileset *curr_ts = ts_grindLungeSword[speedLevel];
	int startFrame = 0;
	showSword = frame < 15;
	//showSword = frame / 3 >= startFrame && frame / 3 <= 5;

	if (showSword)
	{
		swordSprite.setTexture(*curr_ts->texture);
	}

	SetSpriteTexture(GRINDSLASH);

	SetSpriteTile(frame, facingRight);

	Vector2i offset(0, 0);
	switch (speedLevel)
	{
	case 0:
	case 1:
		offset = Vector2i(16, 24);
		break;
	case 2:
		offset = Vector2i(32, 32);
		break;
	}
	//Vector2i offset( 8, -24 );

	if (showSword)
	{
		if (facingRight)
		{
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame / 3 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / 3 - startFrame);
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));


			offset.x = -offset.x;
		}

		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height / 2);
		swordSprite.setPosition(position.x + offset.x, position.y + offset.y);
		swordSprite.setRotation(sprite->getRotation());
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	//sprite->setRotation( 0 );
	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::GRINDSLASH_TransitionToAction(int a)
{

}

void Actor::GRINDSLASH_TimeIndFrameInc()
{

}

void Actor::GRINDSLASH_TimeDepFrameInc()
{

}

int Actor::GRINDSLASH_GetActionLength()
{
	return 16;
}

const char * Actor::GRINDSLASH_GetTilesetName()
{
	return "grind_lunge_96x128.png";
}