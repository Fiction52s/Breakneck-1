#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::STANDATTACK4_Start()
{
	SetActionSuperLevel();

	ActivateSound(PlayerSounds::S_STANDATTACK);
	ResetAttackHit();

	StartStandAttack();
}

void Actor::STANDATTACK4_End()
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

void Actor::STANDATTACK4_Change()
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

void Actor::STANDATTACK4_Update()
{
	SetCurrHitboxes(standHitboxes4[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{

		TryThrowSwordProjectileBasic();

	}

	AttackMovement();
}

void Actor::STANDATTACK4_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword4[speedLevel],
		0, 18, 2, Vector2f(0, 0));
}

void Actor::STANDATTACK4_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::STANDATTACK4_TimeIndFrameInc()
{

}

void Actor::STANDATTACK4_TimeDepFrameInc()
{

}

int Actor::STANDATTACK4_GetActionLength()
{
	return 23 * 2;
}

Tileset * Actor::STANDATTACK4_GetTileset()
{
	return GetActionTileset("stand_att_04_128x128.png");
}