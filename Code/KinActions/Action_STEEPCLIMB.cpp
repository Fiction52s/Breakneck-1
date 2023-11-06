#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::STEEPCLIMB_Start()
{
	V2d norm = ground->Normal();

	if (norm.x > 0)
	{
		facingRight = false;
	}
	else
	{
		facingRight = true;
	}
}

void Actor::STEEPCLIMB_End()
{
	frame = 0;
}

void Actor::STEEPCLIMB_Change()
{
	if (CheckSetToAerialFromNormalWater()) return;

	if (TryPressGrind()) return;

	if (TryGroundBlock()) return;

	if (JumpButtonPressed())
	{
		SetAction(JUMPSQUAT);
		frame = 0;
		return;
	}

	V2d norm = GetGroundedNormal();
	if (reversed)
	{
		if (-norm.y <= -steepThresh || !(approxEquals(offsetX, b.rw)
			|| approxEquals(offsetX, -b.rw)))
		{
			SetAction(LAND2);
			frame = 0;
			if (TryGroundAttack())
			{

			}
			return;
		}

		if (norm.x > 0 && groundSpeed >= 0)
		{
			if (HoldingRelativeUp())
			{
				SetAction(STEEPCLING);
				frame = 0;
				if (SteepClimbAttack())
				{

				}
			}
			else
			{
				SetAction(STEEPSLIDE);
				frame = 0;
				facingRight = true;
				if (SteepSlideAttack())
				{

				}
			}
			return;
		}
		else if (norm.x < 0 && groundSpeed <= 0)
		{
			if (HoldingRelativeUp())
			{
				SetAction(STEEPCLING);
				frame = 0;
				if (SteepClimbAttack())
				{

				}
			}
			else
			{
				SetAction(STEEPSLIDE);
				frame = 0;
				facingRight = false;
				if (SteepSlideAttack())
				{

				}
			}
			return;
		}
	}
	else
	{
		if (norm.y <= -steepThresh || !(approxEquals(offsetX, b.rw)
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

		if (norm.x > 0 && groundSpeed >= 0)
		{
			if (HoldingRelativeUp())
			{
				SetAction(STEEPCLING);
				frame = 0;
				if (SteepClimbAttack())
				{

				}
			}
			else
			{
				SetAction(STEEPSLIDE);
				frame = 0;
				facingRight = true;
				if (SteepSlideAttack())
				{

				}
			}
			return;
		}
		else if (norm.x < 0 && groundSpeed <= 0)
		{
			if (HoldingRelativeUp())
			{
				SetAction(STEEPCLING);
				frame = 0;
				if (SteepClimbAttack())
				{

				}
			}
			else
			{
				SetAction(STEEPSLIDE);
				frame = 0;
				facingRight = false;
				if (SteepSlideAttack())
				{

				}
			}
			return;
		}
	}

	if (SteepClimbAttack())
	{
		return;
	}
}

void Actor::STEEPCLIMB_Update()
{
	SteepClimbMovement();
}

void Actor::STEEPCLIMB_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	int tFrame = frame / 4;
	SetSpriteTile(tFrame, r);

	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	sprite->setPosition(pp.x, pp.y);


	if (frame == 0 * 4 && slowCounter == 1)
	{
		ActivateSound(PlayerSounds::S_CLIMB_STEP1);
	}
	else if (frame == 4 * 4 && slowCounter == 1)
	{
		ActivateSound(PlayerSounds::S_CLIMB_STEP1);
	}

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

void Actor::STEEPCLIMB_TransitionToAction(int a)
{

}

void Actor::STEEPCLIMB_TimeIndFrameInc()
{

}

void Actor::STEEPCLIMB_TimeDepFrameInc()
{

}

int Actor::STEEPCLIMB_GetActionLength()
{
	return 8 * 4;
}

Tileset * Actor::STEEPCLIMB_GetTileset()
{
	return GetActionTileset("steepclimb_96x32.png");
}