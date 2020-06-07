#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::LAND2_Start()
{
	currBBoostCounter = 0;
}

void Actor::LAND2_End()
{
	frame = 0;
}

void Actor::LAND2_Change()
{
	if (hasPowerGrindBall && currInput.Y)//&& !prevInput.Y )
	{
		//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
		if (!reversed || (hasPowerGravReverse && reversed))
		{
			SetActionGrind();
			return;
		}
	}

	if (hasPowerBounce && currInput.X && !bounceFlameOn)
	{
		//bounceGrounded = true;
		BounceFlameOn();
		oldBounceEdge = NULL;
		//break;
	}
	else if (!(hasPowerBounce && currInput.X) && bounceFlameOn)
	{
		BounceFlameOff();
	}

	if (TryJumpSquat()) return;



	if (reversed)
	{
		if (-currNormal.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{

			if (groundSpeed < 0 && currNormal.x > 0 
				|| groundSpeed > 0 && currNormal.x < 0)
			{
				if (groundSpeed > 0)
					facingRight = true;
				else
					facingRight = false;

				SetAction(STEEPCLIMB);

				if (SteepClimbAttack())
				{

				}
				frame = 0;
				return;
			}
			else
			{
				if (currInput.LUp())
				{
					SetAction(STEEPCLING);
					frame = 0;
					facingRight = currNormal.x < 0;
				}
				else
				{
					if (groundSpeed > 0)
						facingRight = true;
					else
						facingRight = false;
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
			if ((currInput.B && !(reversed && (!currInput.LLeft() && !currInput.LRight()))) || !canStandUp)
			{
				SetAction(DASH);

				if (currInput.LLeft())
					facingRight = false;
				else if (currInput.LRight())
					facingRight = true;
			}
			else if (TrySprintOrRun(currNormal))
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

		if (currNormal.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{
			if (groundSpeed > 0 && currNormal.x < 0 || groundSpeed < 0 && currNormal.x > 0)
			{
				if (groundSpeed > 0)
					facingRight = true;
				else
					facingRight = false;
				SetAction(STEEPCLIMB);

				if (SteepClimbAttack())
				{

				}
				frame = 0;
				return;
			}
			else
			{
				if (currNormal.x > 0)
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
			if (currInput.B || !canStandUp)
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
							if (currNormal.x > 0)
							{
								facingRight = true;
							}
							else if (currNormal.x < 0)
							{
								facingRight = false;
							}
						}
					}

				}

				SetAction(DASH);
			}
			else if (TrySprintOrRun(currNormal))
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
	SetSpriteTile(1, r);

	SetGroundedSpriteTransform();
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

Tileset * Actor::LAND2_GetTileset()
{
	return GetActionTileset("land_64x64.png");
}