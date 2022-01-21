#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::STEEPSLIDE_Start()
{
	ActivateRepeatingSound(PlayerSounds::S_STEEPSLIDE, true);

	V2d norm = ground->Normal();

	if (norm.x > 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	/*if (reversed)
	{
		if (norm.x < 0)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
	}
	else
	{
		if (norm.x > 0)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
	}*/
}

void Actor::STEEPSLIDE_End()
{
	frame = 0;
}

void Actor::STEEPSLIDE_Change()
{
	if (TryPressGrind()) return;

	if (JumpButtonPressed())
	{
		if (reversed)
		{
			ReverseSteepSlideJump();
		}
		else
		{
			SetAction(JUMPSQUAT);
			frame = 0;
		}

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
		else
		{
			if (currInput.LUp() && abs(groundSpeed) < steepClingSpeedLimit * 2)
			{
				SetAction(STEEPCLING);
				frame = 0;
				if (SteepClimbAttack())
				{

				}
				facingRight = (currNormal.x < 0);
				return;
			}


			//is steep
			if ((currNormal.x < 0 && groundSpeed > 0)
				|| (currNormal.x > 0 && groundSpeed < 0))
			{
				SetAction(STEEPCLIMB);
				frame = 1;
				if (SteepClimbAttack())
				{

				}
				return;
			}
		}
	}
	else
	{
		if (currNormal.y <= -steepThresh || !(approxEquals(offsetX, b.rw) 
			|| approxEquals(offsetX, -b.rw)))
		{
			//cout << "is it really this wtf" << endl;
			SetAction(LAND2);
			frame = 0;
			if (TryGroundAttack())
			{

			}
			return;
			//not steep
		}
		else
		{
			if (currInput.LUp() && abs(groundSpeed) < steepClingSpeedLimit * 2)
			{
				SetAction(STEEPCLING);
				frame = 0;
				if (SteepClimbAttack())
				{

				}
				facingRight = (currNormal.x < 0);
				return;
			}


			//is steep
			if ((currNormal.x < 0 && groundSpeed > 0) 
				|| (currNormal.x > 0 && groundSpeed < 0))
			{
				SetAction(STEEPCLIMB);
				frame = 1;
				if (SteepClimbAttack())
				{

				}
				return;
			}
		}
	}

	if (SteepSlideAttack())
	{
		return;
	}

	if (DashButtonPressed())
	{
		if (currNormal.x < 0 && (currInput.LRight() || currInput.LUp()))
		{
			SetAction(STEEPCLIMB);
			groundSpeed = steepClimbBoostStart;
			frame = 0;
		}
		else if (currNormal.x > 0 && (currInput.LLeft() || currInput.LUp()))
		{
			SetAction(STEEPCLIMB);
			groundSpeed = -steepClimbBoostStart;
			frame = 0;
		}
		/*else
		{
		action = JUMPSQUAT;
		bufferedAttack = false;
		frame = 0;
		}*/
		return;
	}
}

void Actor::STEEPSLIDE_Update()
{
	double fac = GetGravity() * steepSlideGravFactor;//gravity * 2.0 / 3.0;

	if (currInput.LDown())
	{
		//cout << "fast slide" << endl;

		int numSlideUpgrades = NumUpgradeRange(UPGRADE_W1_INCREASE_STEEP_SLIDE_ACCEL_1, 3);

		double upgradeAmount = steepSlideFastGravFactor * .2;
		double currFactor = steepSlideFastGravFactor + upgradeAmount * numSlideUpgrades;

		

		if (reversed)
		{
			int numCeilingSlideUpgrades = NumUpgradeRange(UPGRADE_W2_INCREASE_CEILING_STEEP_SLIDE_ACCEL_1, 3);
			double ceilingUpgradeAmount = steepSlideFastGravFactor * .2;
			currFactor += numCeilingSlideUpgrades * ceilingUpgradeAmount;
		}

		fac = GetGravity() * currFactor;
	}


	groundSpeed += dot(V2d(0, fac), normalize(ground->v1 - ground->v0)) / slowMultiple;
}

void Actor::STEEPSLIDE_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	double angle = 0;
	if (!approxEquals(abs(offsetX), b.rw))
	{
		if (reversed)
			angle = PI;
		//this should never happen
	}
	else
	{
		angle = atan2(currNormal.x, -currNormal.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	sprite->setPosition(pp.x, pp.y);

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSteepSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSteepSlide, 0, r);

		if (r)
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 20,
				scorpSprite.getLocalBounds().height / 2 + 20);
		}
		else
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 20,
				scorpSprite.getLocalBounds().height / 2 + 20);
		}

		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::STEEPSLIDE_TransitionToAction(int a)
{

}

void Actor::STEEPSLIDE_TimeIndFrameInc()
{

}

void Actor::STEEPSLIDE_TimeDepFrameInc()
{

}

int Actor::STEEPSLIDE_GetActionLength()
{
	return 1;
}

Tileset * Actor::STEEPSLIDE_GetTileset()
{
	return GetActionTileset("steepslide_64x64.png");
}