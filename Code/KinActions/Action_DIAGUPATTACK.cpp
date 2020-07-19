#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DIAGUPATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(S_DIAGUPATTACK);
	ResetAttackHit();
}

void Actor::DIAGUPATTACK_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::DIAGUPATTACK_Change()
{
	BasicAirAttackAction();
}

void Actor::DIAGUPATTACK_Update()
{
	SetCurrHitboxes(diagUpHitboxes[speedLevel], frame / 2);

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

void Actor::DIAGUPATTACK_UpdateSprite()
{
	Tileset *curr_ts = ts_diagUpSword[speedLevel];
	//cout << "fair frame : " << frame / 2 << endl;
	int startFrame = 0;
	showSword = frame / 2 < 11;//frame >= startFrame && frame / 2 <= 9;

	if (showSword)
	{
		diagUpAttackSword.setTexture(*curr_ts->texture);
	}


	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	//Vector2i offset( 32, -16 );
	//Vector2i offset( 0, 0 );

	if (showSword)
	{
		//Vector2i offsets[3];//( 0, 0 );
		//offsets[0] = Vector2i( 40, -32 );
		//offsets[1] = Vector2i( 16, -40 );
		//offsets[2] = Vector2i( 32, -48 );

		Vector2f offset = diagUpSwordOffset[speedLevel];

		if (facingRight)
		{
			diagUpAttackSword.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			diagUpAttackSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}

		diagUpAttackSword.setOrigin(diagUpAttackSword.getLocalBounds().width / 2, diagUpAttackSword.getLocalBounds().height / 2);
		diagUpAttackSword.setPosition(position.x + offset.x, position.y + offset.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::DIAGUPATTACK_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::DIAGUPATTACK_TimeIndFrameInc()
{

}

void Actor::DIAGUPATTACK_TimeDepFrameInc()
{

}

int Actor::DIAGUPATTACK_GetActionLength()
{
	return 11 * 2;
}

Tileset * Actor::DIAGUPATTACK_GetTileset()
{
	return GetActionTileset("airdash_attack_up_96x80.png");
}