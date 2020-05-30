#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::LAND2_Start()
{
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
				if (groundSpeed > 0)
					facingRight = true;
				else
					facingRight = false;
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
			if ((currInput.B && !(reversed && (!currInput.LLeft() && !currInput.LRight()))) || !canStandUp)
			{
				SetActionExpr(DASH);

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

				SetActionExpr(DASH);
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

	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
}