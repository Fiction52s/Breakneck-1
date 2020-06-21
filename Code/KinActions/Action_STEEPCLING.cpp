#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::STEEPCLING_Start()
{
}

void Actor::STEEPCLING_End()
{
	frame = 0;
}

void Actor::STEEPCLING_Change()
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
		SetActionGrind();
		return;
	}

	if (currInput.A && !prevInput.A)
	{
		SetAction(JUMPSQUAT);
		frame = 0;
		return;
	}

	if (reversed)
	{
		if (-currNormal.y <= -steepThresh || !(approxEquals(offsetX, b.rw)
			|| approxEquals(offsetX, -b.rw)))
		{
			SetAction(LAND2);
			frame = 0;
			if (TryGroundAttack())
			{

			}
			return;
		}

		if (currInput.B && !prevInput.B)
		{
			if ((currInput.LRight() || currInput.LUp()) && currNormal.x < 0)
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				groundSpeed = steepClimbBoostStart;
				if (SteepClimbAttack())
				{

				}
			}
			else if ((currInput.LLeft() || currInput.LUp()) && currNormal.x > 0)
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				groundSpeed = -steepClimbBoostStart;
				if (SteepClimbAttack())
				{

				}
			}
		}

		if (!currInput.LUp())
		{
			SetAction(STEEPSLIDE);
			frame = 0;
			facingRight = (currNormal.x > 0 );
			if (SteepSlideAttack())
			{

			}

			return;
		}
		
	}
	else
	{
		if (currNormal.y <= -steepThresh || !(approxEquals(offsetX, b.rw)
			|| approxEquals(offsetX, -b.rw)))
		{
			//cout << "blahzzz" << endl;
			SetAction(LAND2);
			frame = 0;
			if (TryGroundAttack())
			{

			}
			return;
			//not steep
		}


		if ( currInput.B && !prevInput.B )
		{
			if ((currInput.LRight() || currInput.LUp()) && currNormal.x < 0)
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				groundSpeed = steepClimbBoostStart;
				if (SteepClimbAttack())
				{

				}
			}
			else if ((currInput.LLeft() || currInput.LUp()) && currNormal.x > 0)
			{
				SetAction(STEEPCLIMB);
				frame = 0;
				groundSpeed = -steepClimbBoostStart;
				if (SteepClimbAttack())
				{

				}
			}
		}

		if ( !currInput.LUp() )
		{
			SetAction(STEEPSLIDE);
			frame = 0;
			facingRight = (currNormal.x > 0);
			if (SteepSlideAttack())
			{

			}
			return;
		}
	}

	if (SteepClimbAttack())
	{
		return;
	}
}

void Actor::STEEPCLING_Update()
{
	if (reversed)
	{
		groundSpeed += dot(V2d(0, GetGravity() * steepClimbUpFactor), 
			normalize(ground->v1 - ground->v0)) / slowMultiple;
	}
	else
	{
		groundSpeed += dot(V2d(0, GetGravity() * steepClimbUpFactor), 
			normalize(ground->v1 - ground->v0)) / slowMultiple;
	}

	if (currNormal.x > 0 && groundSpeed > steepClingSpeedLimit)
	{
		groundSpeed = steepClingSpeedLimit;
	}
	else if( currNormal.x < 0 && groundSpeed < -steepClingSpeedLimit)
	{
		groundSpeed = -steepClingSpeedLimit;
	}
}

void Actor::STEEPCLING_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	int tFrame = 0;
	SetSpriteTile(tFrame, r);

	double angle = atan2(currNormal.x, -currNormal.y);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	sprite->setPosition(pp.x, pp.y);

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpClimb->texture);

		SetSpriteTile(&scorpSprite, ts_scorpClimb, tFrame, r);

		if (r)
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 30,
				scorpSprite.getLocalBounds().height / 2 + 25);
		}
		else
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 30,
				scorpSprite.getLocalBounds().height / 2 + 25);
		}

		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::STEEPCLING_TransitionToAction(int a)
{

}

void Actor::STEEPCLING_TimeIndFrameInc()
{

}

void Actor::STEEPCLING_TimeDepFrameInc()
{

}

int Actor::STEEPCLING_GetActionLength()
{
	return 1;
}

Tileset * Actor::STEEPCLING_GetTileset()
{
	return STEEPCLIMB_GetTileset();
}