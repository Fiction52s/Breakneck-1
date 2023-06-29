#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::UPTILT1_Start()
{
	SetActionSuperLevel();

	ActivateSound(PlayerSounds::S_UAIR);
	ResetAttackHit();
	StartUpTilt();
}

void Actor::UPTILT1_End()
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

void Actor::UPTILT1_Change()
{
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

void Actor::UPTILT1_Update()
{
	SetCurrHitboxes(standHitboxes1[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		TryThrowSwordProjectileBasic();
	}

	AttackMovement();
}

void Actor::UPTILT1_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_standAttackSword2[speedLevel],
		0, 10, 2, Vector2f(0, 0));
}

void Actor::UPTILT1_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::UPTILT1_TimeIndFrameInc()
{

}

void Actor::UPTILT1_TimeDepFrameInc()
{

}

int Actor::UPTILT1_GetActionLength()
{
	return 16 * 2;
}

Tileset * Actor::UPTILT1_GetTileset()
{
	return GetActionTileset("stand_att_02_64x64.png");
}