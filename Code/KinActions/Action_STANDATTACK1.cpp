#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::STANDATTACK1_Start()
{
	SetActionSuperLevel();

	ActivateSound(S_STANDATTACK);
	ResetAttackHit();

	StartStandAttack();
}

void Actor::STANDATTACK1_End()
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

void Actor::STANDATTACK1_Change()
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

void Actor::STANDATTACK1_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::STANDATTACK1_UpdateSprite()
{
	int startFrame = 0;

	showSword = true;

	Tileset *curr_ts = ts_standAttackSword[speedLevel];

	if (showSword)
	{
		standAttackSword.setTexture(*curr_ts->texture);
	}

	Vector2f offset = standSwordOffset[speedLevel];

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame / 2, r);

	if (showSword)
	{
		if (r)
		{
			standAttackSword.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			standAttackSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}
	}


	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		standAttackSword.setOrigin(standAttackSword.getLocalBounds().width / 2, standAttackSword.getLocalBounds().height/2);
		standAttackSword.setRotation(angle / PI * 180);
	}

	SetGroundedSpriteTransform();

	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);

	pos += truDir * (double)offset.x;
	pos += -trueNormal * (double)(offset.y - sprite->getLocalBounds().height / 2);


	standAttackSword.setPosition(pos.x, pos.y);
}

void Actor::STANDATTACK1_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::STANDATTACK1_TimeIndFrameInc()
{

}

void Actor::STANDATTACK1_TimeDepFrameInc()
{

}

int Actor::STANDATTACK1_GetActionLength()
{
	return 12 * 2;
}

Tileset * Actor::STANDATTACK1_GetTileset()
{
	return GetActionTileset("stand_att_01_64x64.png");
}