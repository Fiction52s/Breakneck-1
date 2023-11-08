#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::LAND2_Start()
{
	holdJump = false;
	holdDouble = false;
	currBBoostCounter = 0;
	if (TryGroundBlock()) return;
}

void Actor::LAND2_End()
{
	frame = 0;
}

void Actor::LAND2_Change()
{
	if (TryGroundBlock()) return;

	if (CanBufferGrind())
	{
		//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
		if (!reversed || (HasUpgrade(UPGRADE_POWER_GRAV) && reversed))
		{
			SetActionGrind();
			return;
		}
	}

	if (TryJumpSquat()) return;

	V2d norm = GetGroundedNormal();

	if (reversed)
	{
		if (-norm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{

			if (groundSpeed < 0 && norm.x > 0
				|| groundSpeed > 0 && norm.x < 0)
			{
				SetAction(STEEPCLIMB);

				if (SteepClimbAttack())
				{

				}
				frame = 0;
				return;
			}
			else
			{
				if (HoldingRelativeUp())
				{
					SetAction(STEEPCLING);
					frame = 0;
					facingRight = norm.x < 0;
				}
				else
				{
					SetAction(STEEPSLIDE);
					if (SteepSlideAttack())
					{

					}
					frame = 0;
				}
				
				return;
			}

		}
		else
		{
			if ((DashButtonHeld() && !(reversed && (!currInput.LLeft() && !currInput.LRight()))) || !canStandUp)
			{
				SetAction(DASH);

				if (currInput.LLeft())
					facingRight = false;
				else if (currInput.LRight())
					facingRight = true;
			}
			else if (TrySprintOrRun())
			{

			}
			else if (TrySlideBrakeOrStand())
			{

			}

			if (TryGroundAttack())
			{
				return;
			}
		}
	}
	else
	{

		if (norm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{
			if (groundSpeed > 0 && norm.x < 0 || groundSpeed < 0 && norm.x > 0)
			{
				SetAction(STEEPCLIMB);

				if (SteepClimbAttack())
				{

				}
				frame = 0;
				return;
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

				SetAction(STEEPSLIDE);

				if (SteepSlideAttack())
				{

				}

				frame = 0;
				return;
			}

		}
		else
		{
			if (DashButtonHeld() || !canStandUp)
			{
				if (currInput.LLeft())
					facingRight = false;
				else if (currInput.LRight())
					facingRight = true;
				else
				{
					if (currInput.LDown())
					{
						if (groundSpeed > 0)//velocity.x > 0 )
						{
							facingRight = true;
						}
						else if (groundSpeed < 0)//velocity.x < 0 )
						{
							facingRight = false;
						}
						else
						{
							if (norm.x > 0)
							{
								facingRight = true;
							}
							else if (norm.x < 0)
							{
								facingRight = false;
							}
						}
					}

				}

				SetAction(DASH);
			}
			else if (TrySprintOrRun())
			{

			}
			else if (TrySlideBrakeOrStand())
			{
			}

			if (TryGroundAttack())
				return;
		}
	}
}

void Actor::LAND2_Update()
{
}

void Actor::LAND2_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(2, r);

	SetGroundedSpriteTransform();

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::LAND2_TransitionToAction(int a)
{

}

void Actor::LAND2_TimeIndFrameInc()
{

}

void Actor::LAND2_TimeDepFrameInc()
{

}

int Actor::LAND2_GetActionLength()
{
	return 1;
}

const char * Actor::LAND2_GetTilesetName()
{
	return "land_64x64.png";
}