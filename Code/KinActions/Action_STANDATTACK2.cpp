#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::STANDATTACK2_Start()
{
	SetActionSuperLevel();

	ActivateSound(S_STANDATTACK);
	ResetAttackHit();
	StartStandAttack();
}

void Actor::STANDATTACK2_End()
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

void Actor::STANDATTACK2_Change()
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

void Actor::STANDATTACK2_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::STANDATTACK2_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword2[speedLevel],
		0, 10, 2, Vector2f(0, 0));
}

void Actor::STANDATTACK2_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::STANDATTACK2_TimeIndFrameInc()
{

}

void Actor::STANDATTACK2_TimeDepFrameInc()
{

}

int Actor::STANDATTACK2_GetActionLength()
{
	return 16 * 2;
}

Tileset * Actor::STANDATTACK2_GetTileset()
{
	return GetActionTileset("stand_att_02_64x64.png");
}