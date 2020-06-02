#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::LAND_Start()
{
	currBBoostCounter = 0;
}

void Actor::LAND_End()
{
	frame = 0;
}

void Actor::LAND_Change()
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

			if (groundSpeed < 0 && currNormal.x > 0 || groundSpeed > 0 && currNormal.x < 0)
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

void Actor::LAND_Update()
{
}

void Actor::LAND_UpdateSprite()
{
	SetSpriteTexture(LAND);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);


	double angle = GroundedAngle();


	

	V2d pp = ground->GetPosition(edgeQuantity);

	SetGroundedSpriteTransform();

	if (frame == 0 && slowCounter == 1)
	{
		V2d fxPos;
		if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		{
			fxPos = V2d(pp.x + offsetX, pp.y);
			fxPos += V2d(0, -1) * 48.0;
		}
		else
		{
			fxPos = pp;
			fxPos += currNormal * 48.0;
		}

		//cout << "activating" << endl;
		switch (speedLevel)
		{
		case 0:
			ActivateEffect(EffectLayer::IN_FRONT, ts_fx_land[0], fxPos, false, angle, 8, 2, facingRight);
			break;
		case 1:
			ActivateEffect(EffectLayer::IN_FRONT, ts_fx_land[1], fxPos, false, angle, 8, 2, facingRight);
			break;
		case 2:
			ActivateEffect(EffectLayer::IN_FRONT, ts_fx_land[2], fxPos, false, angle, 9, 2, facingRight);
			break;
		}

	}
}

void Actor::LAND_TransitionToAction(int a)
{

}

void Actor::LAND_TimeIndFrameInc()
{

}

void Actor::LAND_TimeDepFrameInc()
{

}

int Actor::LAND_GetActionLength()
{
	return 1;
}

Tileset * Actor::LAND_GetTileset()
{
	return GetActionTileset("land_64x64.png");
}