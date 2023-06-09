#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::UPTILT2_Start()
{
	SetActionSuperLevel();

	ActivateSound(PlayerSounds::S_UAIR);
	ResetAttackHit();

	StartUpTilt();
}

void Actor::UPTILT2_End()
{
	if (currInput.LLeft() || currInput.LRight())
	{
		if (DashButtonHeld())
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
		if (TryPressGrind()) return;

		if (JumpButtonPressed() || pauseBufferedJump)
		{
			SetAction(JUMPSQUAT);
			frame = 0;
			return;
		}

		if (TryGroundAttack())
		{
			return;
		}

		if (pauseBufferedDash || DashButtonPressed())
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
	SetCurrHitboxes(standHitboxes1[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{

		TryThrowSwordProjectileBasic();

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