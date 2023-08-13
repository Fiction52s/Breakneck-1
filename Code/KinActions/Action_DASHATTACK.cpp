#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(PlayerSounds::S_STANDATTACK);
	ResetAttackHit();
	StartDashAttack();
}

void Actor::DASHATTACK_End()
{
	if (currInput.LLeft() || currInput.LRight())
	{
		if (DashButtonHeld())
		{
			SetAction(DASH);
			//action = DASH;
			//re->Reset();
			//re1->Reset();
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
		//SetAction(STAND);
	}
	frame = 0;
}

void Actor::DASHATTACK_Change()
{
	if (TryFloorRailDropThrough()) return;

	if (CanCancelAttack() || frame > 14)
	{
		//if (TryBufferGrind()) return;
		//if (TryPressGrind()) return;

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
			if (currAttackHit)
			{
				if (standNDashBoostCurr == 0)
				{
					standNDashBoost = true;
					standNDashBoostCurr = standNDashBoostCooldown;
				}
			}
			SetAction(DASH);
			frame = 0;
			return;
		}

		if (TryBufferGrind()) return;

		if (TryGroundBlock()) return;

	}
}


////button-based inputs
//
//if (TryGroundBlock()) return true;
//
//if (TryFloorRailDropThrough()) return true;
//
//if (TryPressGrind()) return true;
//
//if (TryJumpSquat()) return true;
//
//if (TryGroundAttack()) return true;
//
//if (TryDash()) return true;

void Actor::DASHATTACK_Update()
{
	SetCurrHitboxes(dashHitboxes1[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		TryThrowSwordProjectileBasic();

	}

	AttackMovement();
}

void Actor::DASHATTACK_UpdateSprite()
{
	UpdateGroundedAttackSprite(action, ts_dashAttackSword[speedLevel],
		0, 8, 2, Vector2f(0, 0));
}

void Actor::DASHATTACK_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::DASHATTACK_TimeIndFrameInc()
{

}

void Actor::DASHATTACK_TimeDepFrameInc()
{

}

int Actor::DASHATTACK_GetActionLength()
{
	return 14 * 2;
}

Tileset * Actor::DASHATTACK_GetTileset()
{
	return GetActionTileset("dash_att_01_128x64.png");
}