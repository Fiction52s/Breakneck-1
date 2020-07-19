#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASHATTACK_Start()
{
	SetActionSuperLevel();
	ActivateSound(S_STANDATTACK);
	ResetAttackHit();
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
	if (CanCancelAttack() || frame > 14)//&& frame > 0 )
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
	int startFrame = 0;
	showSword = true;

	Tileset *curr_ts = ts_dashAttackSword[speedLevel];

	if (frame >= 8 * 2)
		showSword = false;

	if (showSword)
	{
		dashAttackSword.setTexture(*curr_ts->texture);
	}
	Vector2f offset = dashAttackSwordOffset[speedLevel];

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame/2, r);

	if (showSword)
	{
		if (r)
		{
			dashAttackSword.setTextureRect(curr_ts->GetSubRect(frame/2 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame/2 - startFrame);
			dashAttackSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}
	}


	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		dashAttackSword.setOrigin(dashAttackSword.getLocalBounds().width / 2, dashAttackSword.getLocalBounds().height/2);
		dashAttackSword.setRotation(angle / PI * 180);
		//standingNSword1.setPosition( position.x + offset.x, position.y + offset.y );
	}

	SetGroundedSpriteTransform();

	//V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y ) + V2d( offset.x * cos( angle ) + offset.y * sin( angle ), 
	//offset.x * -sin( angle ) +  offset.y * cos( angle ) );
	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);//normalize( ground->v1 - ground->v0 );

	pos += truDir * (double)offset.x;
	pos += -trueNormal * (double)( offset.y - sprite->getLocalBounds().height / 2);


	dashAttackSword.setPosition(pos.x, pos.y);
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