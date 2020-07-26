#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::FAIR_Start()
{
	SetActionSuperLevel();
	if (currActionSuperLevel == 2)
	{
		SetAction(SUPERBIRD);
	}
	else
	{
		ActivateSound(S_FAIR1);
		ResetAttackHit();
	}
}

void Actor::FAIR_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::FAIR_Change()
{
	BasicAirAttackAction();
}

void Actor::FAIR_Update()
{
	CheckHoldJump();


	SetCurrHitboxes(fairHitboxes[speedLevel], frame);

	if (frame == 0 && slowCounter == 1)
	{
		
		//fairSound.play();
	}
	if (wallJumpFrameCounter >= wallJumpMovementLimit)
	{
		//cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << endl;

		AirMovement();
	}
}

void Actor::FAIR_UpdateSprite()
{
	Tileset *curr_ts = ts_fairSword[speedLevel];
	//cout << "fair frame : " << frame / 2 << endl;
	int startFrame = 0;
	showSword = true;//frame >= startFrame && frame / 2 <= 9;

	if (showSword)
	{
		fairSword.setTexture(*curr_ts->texture);
	}

	//Vector2i offset( 32, -16 );
	Vector2i offset(0, 0);

	SetSpriteTexture(action);

	SetSpriteTile(frame, facingRight);

	if (showSword)
	{
		if (facingRight)
		{
			fairSword.setTextureRect(curr_ts->GetSubRect(frame - startFrame));
		}
		else
		{
			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame - startFrame);
			//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
			fairSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}

		fairSword.setOrigin(fairSword.getLocalBounds().width / 2, fairSword.getLocalBounds().height / 2);
		fairSword.setPosition(position.x + offset.x, position.y + offset.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::FAIR_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::FAIR_TimeIndFrameInc()
{

}

void Actor::FAIR_TimeDepFrameInc()
{

}

int Actor::FAIR_GetActionLength()
{
	return 8 * 2;
}

Tileset * Actor::FAIR_GetTileset()
{
	return GetActionTileset("fair_80x80.png");
}