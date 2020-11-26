#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::UPTILT3_Start()
{
	SetActionSuperLevel();

	ActivateSound(S_UAIR);
	ResetAttackHit();

	StartUpTilt();
}

void Actor::UPTILT3_End()
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

void Actor::UPTILT3_Change()
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

void Actor::UPTILT3_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::UPTILT3_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword4[speedLevel],
		0, 18, 2, Vector2f(0, 0));
}

void Actor::UPTILT3_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::UPTILT3_TimeIndFrameInc()
{

}

void Actor::UPTILT3_TimeDepFrameInc()
{

}

int Actor::UPTILT3_GetActionLength()
{
	return 23 * 2;
}

Tileset * Actor::UPTILT3_GetTileset()
{
	return GetActionTileset("stand_att_04_128x128.png");
}