#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::AIRDASHFORWARDATTACK_Start()
{
	SetActionSuperLevel();
	if (currActionSuperLevel == 2)
	{
		SetAction(SUPERBIRD);
	}
	else
	{
		ActivateSound(PlayerSounds::S_FAIR1);
		ResetAttackHit();
	}
}

void Actor::AIRDASHFORWARDATTACK_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::AIRDASHFORWARDATTACK_Change()
{
	if (bouncedFromKill)
	{
		SetAction(BOUNCEAIR);
		if (facingRight)
		{
			BounceFloaterBoost(V2d(1, 0));
			facingRight = false;
		}
		else
		{
			BounceFloaterBoost(V2d(-1, 0));
			facingRight = true;
		}

	}

	bool changed = BasicAirAttackAction();
	if (!changed)
	{
		TryWallJump();
	}
}

void Actor::AIRDASHFORWARDATTACK_Update()
{
	CheckHoldJump();


	SetCurrHitboxes(fairHitboxes[speedLevel], frame);

	if (frame == 0 && slowCounter == 1)
	{
		TryThrowSwordProjectileBasic();
	}
	if (wallJumpFrameCounter >= wallJumpMovementLimit)
	{
		//cout << "wallJumpFrameCounter: " << wallJumpFrameCounter << endl;

		AirMovement();
	}
}

void Actor::AIRDASHFORWARDATTACK_UpdateSprite()
{
	Tileset *curr_ts = ts_fairSword[speedLevel];
	int startFrame = 0;
	showSword = true;//frame >= startFrame && frame / 2 <= 9;

	if (showSword)
	{
		swordSprite.setTexture(*curr_ts->texture);
	}

	//Vector2i offset( 32, -16 );
	Vector2i offset(0, 0);

	SetSpriteTexture(action);

	SetSpriteTile(frame, facingRight);

	if (showSword)
	{
		if (facingRight)
		{
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame - startFrame));
		}
		else
		{
			offset.x = -offset.x;

			sf::IntRect irSword = curr_ts->GetSubRect(frame - startFrame);
			//sf::IntRect irSword = ts_AIRDASHFORWARDATTACKSword1->GetSubRect( frame - startFrame );
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

void Actor::AIRDASHFORWARDATTACK_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::AIRDASHFORWARDATTACK_TimeIndFrameInc()
{

}

void Actor::AIRDASHFORWARDATTACK_TimeDepFrameInc()
{

}

int Actor::AIRDASHFORWARDATTACK_GetActionLength()
{
	return 8 * 2;
}

Tileset * Actor::AIRDASHFORWARDATTACK_GetTileset()
{
	return GetActionTileset("fair_80x80.png");
}