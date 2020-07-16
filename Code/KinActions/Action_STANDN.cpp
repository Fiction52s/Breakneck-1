#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::STANDN_Start()
{
	SetActionSuperLevel();
}

void Actor::STANDN_End()
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

void Actor::STANDN_Change()
{
	if (currAttackHit)//&& frame > 0 )
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

void Actor::STANDN_Update()
{
	SetCurrHitboxes(standHitboxes[speedLevel], frame / 2);

	if (frame == 0 && slowCounter == 1)
	{
		ActivateSound(S_STANDATTACK);
		currAttackHit = false;
	}

	AttackMovement();
}

void Actor::STANDN_UpdateSprite()
{
	int startFrame = 0;
	showSword = true;

	Tileset *curr_ts = ts_standingNSword[speedLevel];

	if (showSword)
	{
		standingNSword.setTexture(*curr_ts->texture);
	}
	//Vector2i offset( 24, -16 );
	//Vector2i offset( 24, 0 );
	//Vector2i offset( 32, 0 );
	//Vector2i offset( 0, -16 );
	Vector2f offset = standSwordOffset[speedLevel];

	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(frame / 2, r);

	if (showSword)
	{
		if (r)
		{
			standingNSword.setTextureRect(curr_ts->GetSubRect(frame / 2 - startFrame));
		}
		else
		{
			sf::IntRect irSword = curr_ts->GetSubRect(frame / 2 - startFrame);
			standingNSword.setTextureRect(sf::IntRect(irSword.left + irSword.width,
				irSword.top, -irSword.width, irSword.height));

			offset.x = -offset.x;
		}
	}


	V2d trueNormal;
	double angle = GroundedAngleAttack(trueNormal);

	if (showSword)
	{
		standingNSword.setOrigin(standingNSword.getLocalBounds().width / 2, standingNSword.getLocalBounds().height);
		standingNSword.setRotation(angle / PI * 180);
		//standingNSword1.setPosition( position.x + offset.x, position.y + offset.y );
	}

	SetGroundedSpriteTransform();

	//V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y ) + V2d( offset.x * cos( angle ) + offset.y * sin( angle ), 
	//offset.x * -sin( angle ) +  offset.y * cos( angle ) );
	V2d pos = V2d(sprite->getPosition().x, sprite->getPosition().y);
	V2d truDir(-trueNormal.y, trueNormal.x);//normalize( ground->v1 - ground->v0 );

	pos += truDir * (double)offset.x;
	pos += -trueNormal * (double)offset.y;


	standingNSword.setPosition(pos.x, pos.y);
}

void Actor::STANDN_TransitionToAction(int a)
{
	ResetSuperLevel();
}

void Actor::STANDN_TimeIndFrameInc()
{

}

void Actor::STANDN_TimeDepFrameInc()
{

}

int Actor::STANDN_GetActionLength()
{
	return 8 * 2;
}

Tileset * Actor::STANDN_GetTileset()
{
	return GetActionTileset("standn_96x64.png");
}