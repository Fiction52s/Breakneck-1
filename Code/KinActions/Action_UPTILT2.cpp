#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::UPTILT2_Start()
{
	SetActionSuperLevel();

	ActivateSound(S_UAIR);
	ResetAttackHit();

	StartStandAttack();
}

void Actor::UPTILT2_End()
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

void Actor::UPTILT2_Change()
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

void Actor::UPTILT2_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::UPTILT2_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword3[speedLevel],
		0, 11, 2, Vector2f(0, 0));
}

void Actor::UPTILT2_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::UPTILT2_TimeIndFrameInc()
{

}

void Actor::UPTILT2_TimeDepFrameInc()
{

}

int Actor::UPTILT2_GetActionLength()
{
	return 14 * 2;
}

Tileset * Actor::UPTILT2_GetTileset()
{
	return GetActionTileset("stand_att_03_64x64.png");
}