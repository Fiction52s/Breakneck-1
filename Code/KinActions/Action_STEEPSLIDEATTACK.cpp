#include "Actor.h"
#include <assert.h>
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::STEEPSLIDEATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(PlayerSounds::S_STANDATTACK);
	ResetAttackHit();

	V2d norm = ground->Normal();
	if (norm.x > 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}
}

void Actor::STEEPSLIDEATTACK_End()
{
	SetAction(STEEPSLIDE);
	frame = 0;
}

void Actor::STEEPSLIDEATTACK_Change()
{
	if (CheckSetToAerialFromNormalWater()) return;

	if (TryFloorRailDropThrough()) return;

	if (CanCancelAttack())//&& frame > 0 )
	{
		if (TryBufferGrind()) return;

		if (TryGroundBlock()) return;

		if (JumpButtonPressed() || pauseBufferedJump)
		{
			SetAction(JUMPSQUAT);
			frame = 0;
			return;
		}

		if (TryGroundAttack())
			return;
		//if (SteepSlideAttack())
		//	return;

		V2d norm = GetGroundedNormal();

		if (DashButtonPressed())
		{
			if (norm.x < 0 && currInput.LRight())
			{
				SetAction(STEEPCLIMB);
				groundSpeed = steepClimbBoostStart;
				frame = 0;
			}
			else if (norm.x > 0 && currInput.LLeft())
			{
				SetAction(STEEPCLIMB);
				groundSpeed = -steepClimbBoostStart;
				frame = 0;
			}
			return;
		}
	}
}

void Actor::STEEPSLIDEATTACK_Update()
{
	if (frame / 2 > 7)
	{
		assert(0);
	}
	SetCurrHitboxes(steepSlideHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{

		TryThrowSwordProjectileBasic();

	}

	AttackMovement();
	//SteepSlideMovement();
}

void Actor::STEEPSLIDEATTACK_UpdateSprite()
{
	int startFrame = 0;
	showSword = true;//frame / 2 >= startFrame && frame / 2 <= 7;
	Tileset *curr_ts = ts_steepSlideAttackSword[speedLevel];

	if (showSword)
	{
		swordSprite.setTexture(*curr_ts->texture);
	}

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame / 2, r);


	Vector2f offset = slideAttackOffset[speedLevel];

	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		if (r)
		{
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / 3 - startFrame);
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}

		swordSprite.setTexture(*curr_ts->texture);
		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2, swordSprite.getLocalBounds().height);
		swordSprite.setRotation(angle / PI * 180);
	}


	SetGroundedSpriteTransform();

	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);//normalize( ground->v1 - ground->v0 );

	pos += trueNormal * (double)offset.y;
	pos += truDir * (double)offset.x;

	swordSprite.setPosition(pos.x, pos.y);

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSteepSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSteepSlide, 0, r);

		if (r)
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 20,
				scorpSprite.getLocalBounds().height / 2 + 20);
		}
		else
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 20,
				scorpSprite.getLocalBounds().height / 2 + 20);
		}

		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::STEEPSLIDEATTACK_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::STEEPSLIDEATTACK_TimeIndFrameInc()
{

}

void Actor::STEEPSLIDEATTACK_TimeDepFrameInc()
{

}

int Actor::STEEPSLIDEATTACK_GetActionLength()
{
	return 16;
}

Tileset * Actor::STEEPSLIDEATTACK_GetTileset()
{
	return GetActionTileset("steep_att_128x64.png");
}