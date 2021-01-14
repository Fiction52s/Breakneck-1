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

void Actor::STANDATTACK1_Change()
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

void Actor::STANDATTACK1_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		
		TryThrowSwordProjectileBasic();
		
	}

	AttackMovement();
}

void Actor::STANDATTACK1_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword[speedLevel],
		0, -1, 2, Vector2f(0, 0));
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