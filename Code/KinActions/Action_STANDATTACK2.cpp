#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::STANDATTACK2_Start()
{
	SetActionSuperLevel();

	ActivateSound(PlayerSounds::S_STANDATTACK);
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
	if (CheckSetToAerialFromNormalWater()) return;

	if (TryFloorRailDropThrough()) return;

	if (CanCancelAttack() || frame > 14)
	{
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

		if (TryBufferGrind()) return;

		if (TryGroundBlock()) return;
	}
}

void Actor::STANDATTACK2_Update()
{
	SetCurrHitboxes(standHitboxes2[speedLevel], frame / 2);


	if (frame == 0 && slowCounter == 1)
	{

		TryThrowSwordProjectileBasic();

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

const char * Actor::STANDATTACK2_GetTilesetName()
{
	return "stand_att_02_96x64.png";
}