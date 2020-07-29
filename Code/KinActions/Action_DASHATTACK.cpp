#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(S_STANDATTACK);
	ResetAttackHit();
	StartDashAttack();
}

void Actor::DASHATTACK_End()
{
	if (currInput.LLeft() || currInput.LRight())
	{
		if (currInput.B)
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
	}
}

void Actor::DASHATTACK_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

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