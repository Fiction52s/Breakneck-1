#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::STANDATTACK3_Start()
{
	SetActionSuperLevel();

	ActivateSound(S_STANDATTACK);
	ResetAttackHit();

	StartStandAttack();
}

void Actor::STANDATTACK3_End()
{
	if (currInput.LLeft() || currInput.LRight())
	{
		if (currInput.B)
		{
			SetAction(DASH);
		}
		else
		{
			SetAction(RUN);
		}
		facingRight = currInput.LRight();
	}
	else
	{
		SetAction(BRAKE);
	}
	frame = 0;
}

void Actor::STANDATTACK3_Change()
{
	if (CanCancelAttack() || frame > 14)
	{
		if (HasUpgrade(UPGRADE_POWER_BOUNCE) && currInput.X && !bounceFlameOn)
		{
			BounceFlameOn();
		}
		else if (!(HasUpgrade(UPGRADE_POWER_BOUNCE) && currInput.X) && bounceFlameOn)
		{
			//bounceGrounded = false;
			BounceFlameOff();
		}

		if (HasUpgrade(UPGRADE_POWER_GRIND) && currInput.Y && !prevInput.Y)
		{
			BounceFlameOff();
			SetActionGrind();
			//dashStartSound.setLoop( false );
			////runTappingSound.stop();
			return;
		}

		if ((currInput.A && !prevInput.A) || pauseBufferedJump)
		{
			SetAction(JUMPSQUAT);
			frame = 0;
			return;
		}

		if (TryGroundAttack())
		{
			return;
		}

		if (pauseBufferedDash || (currInput.B && !prevInput.B))
		{
			if (standNDashBoostCurr == 0)
			{
				standNDashBoost = true;
				standNDashBoostCurr = standNDashBoostCooldown;
			}
			SetAction(DASH);
			frame = 0;
			return;
		}
	}
}

void Actor::STANDATTACK3_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::STANDATTACK3_UpdateSprite()
{
	int startFrame = 0;

	showSword = true;

	if (frame >= 11 * 2)
		showSword = false;

	Tileset *curr_ts = ts_standAttackSword3[speedLevel];

	if (showSword)
	{
		swordSprite.setTexture(*curr_ts->texture);
	}

	Vector2f offset = standSwordOffset[speedLevel];

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame / 2, r);

	if (showSword)
	{
		if (r)
		{
			swordSprite.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			swordSprite.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}
	}


	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		swordSprite.setOrigin(swordSprite.getLocalBounds().width / 2,
			swordSprite.getLocalBounds().height/2);
		swordSprite.setRotation(angle / PI * 180);
	}

	SetGroundedSpriteTransform();

	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);

	pos += truDir * (double)offset.x;
	pos += -trueNormal * (double)(offset.y - sprite->getLocalBounds().height / 2);


	swordSprite.setPosition(pos.x, pos.y);
}

void Actor::STANDATTACK3_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::STANDATTACK3_TimeIndFrameInc()
{

}

void Actor::STANDATTACK3_TimeDepFrameInc()
{

}

int Actor::STANDATTACK3_GetActionLength()
{
	return 14 * 2;
}

Tileset * Actor::STANDATTACK3_GetTileset()
{
	return GetActionTileset("stand_att_03_64x64.png");
}