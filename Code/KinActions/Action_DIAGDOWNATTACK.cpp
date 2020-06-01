#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DIAGDOWNATTACK_Start()
{

}

void Actor::DIAGDOWNATTACK_End()
{
	SetActionExpr(JUMP);
	frame = 1;
}

void Actor::DIAGDOWNATTACK_Change()
{
	BasicAirAttackAction();
}

void Actor::DIAGDOWNATTACK_Update()
{
	SetCurrHitboxes(diagDownHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		ActivateSound(S_DIAGDOWNATTACK);
		currAttackHit = false;
		//fairSound.play();
	}
	if (wallJumpFrameCounter >= wallJumpMovementLimit)
	{
		//cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << endl;

		AirMovement();
	}
}

void Actor::DIAGDOWNATTACK_UpdateSprite()
{
	Tileset *curr_ts = ts_diagDownSword[speedLevel];
	//cout << "fair frame : " << frame / 2 << endl;
	int startFrame = 0;
	showSword = frame / 2 < 11;//frame >= startFrame && frame / 2 <= 9;

	if (showSword)
	{
		diagDownAttackSword.setTexture(*curr_ts->texture);
	}


	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	//Vector2i offset( 32, -16 );


	if (showSword)
	{

		//Vector2i offsets[3];//( 0, 0 );
		//offsets[0] = Vector2i( 32, 24 );
		//offsets[1] = Vector2i( 16, 32 );
		//offsets[2] = Vector2i( 16, 64 );

		Vector2f offset = diagDownSwordOffset[speedLevel];

		if (facingRight)
		{
			diagDownAttackSword.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{

			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
			diagDownAttackSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}

		diagDownAttackSword.setOrigin(diagDownAttackSword.getLocalBounds().width / 2, diagDownAttackSword.getLocalBounds().height / 2);
		diagDownAttackSword.setPosition(position.x + offset.x, position.y + offset.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::DIAGDOWNATTACK_TransitionToAction(int a)
{

}

void Actor::DIAGDOWNATTACK_TimeIndFrameInc()
{

}

void Actor::DIAGDOWNATTACK_TimeDepFrameInc()
{

}

int Actor::DIAGDOWNATTACK_GetActionLength()
{
	return 1;
}

Tileset * Actor::DIAGDOWNATTACK_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}