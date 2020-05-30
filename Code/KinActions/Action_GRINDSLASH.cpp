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
		grindLungeSword.setTexture(*curr_ts->texture);
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
			grindLungeSword.setTextureRect(curr_ts->GetSubRect(frame / 3 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / 3 - startFrame);
			grindLungeSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));


			offset.x = -offset.x;
		}

		grindLungeSword.setOrigin(grindLungeSword.getLocalBounds().width / 2, grindLungeSword.getLocalBounds().height / 2);
		grindLungeSword.setPosition(position.x + offset.x, position.y + offset.y);
		grindLungeSword.setRotation(sprite->getRotation());
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	//sprite->setRotation( 0 );
	if (scorpOn)
		SetAerialScorpSprite();
}