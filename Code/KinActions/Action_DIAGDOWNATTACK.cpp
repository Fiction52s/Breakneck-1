#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DIAGDOWNATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(S_DIAGDOWNATTACK);
	ResetAttackHit();
}

void Actor::DIAGDOWNATTACK_End()
{
	SetAction(JUMP);
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
		
		TryThrowSwordProjectileBasic();
		
		
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
		swordSprite.setTexture(*curr_ts->texture);
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
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{

			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			//sf::IntRect irSword = ts_fairSword1->GetSubRect( frame - startFrame );
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));
		}

		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height / 2);
		swordSprite.setPosition(position.x + offset.x, position.y + offset.y);
		swordSprite.setRotation(0);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::DIAGDOWNATTACK_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::DIAGDOWNATTACK_TimeIndFrameInc()
{

}

void Actor::DIAGDOWNATTACK_TimeDepFrameInc()
{

}

int Actor::DIAGDOWNATTACK_GetActionLength()
{
	return 11 * 2;
}

Tileset * Actor::DIAGDOWNATTACK_GetTileset()
{
	return GetActionTileset("airdash_attack_down_64x64.png");
}