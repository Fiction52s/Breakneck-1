#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::STANDATTACK3_Start()
{
	SetActionSuperLevel();

	ActivateSound(PlayerSounds::S_STANDATTACK);
	ResetAttackHit();

	StartStandAttack();
}

void Actor::STANDATTACK3_End()
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

void Actor::STANDATTACK3_Change()
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

void Actor::STANDATTACK3_Update()
{
	SetCurrHitboxes(standHitboxes3[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
	}

	AttackMovement();
}

void Actor::STANDATTACK3_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword3[speedLevel],
		0, 11, 2, Vector2f(0, 0));
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