#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::LAND_Start()
{
	if (currInput.IsLeftNeutral() && !IsOnSteepGround() )//!ground->IsSteepGround())
	{
		groundSpeed = 0; //recently added cuz of annoying slide during land anim
	}

	holdJump = false;
	holdDouble = false;
	currBBoostCounter = 0;
	if (TryGroundBlock()) return;
}

void Actor::LAND_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::LAND_Change()
{
	if (TryGroundBlock()) return;

	if ( CanBufferGrind() )
	{
		//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
		if (!reversed || (IsOptionOn(UPGRADE_POWER_GRAV) && reversed))
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

			if (groundSpeed < 0 && norm.x > 0 || groundSpeed > 0 && norm.x < 0)
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
			V2d norm = GetGroundedNormal();
			fxPos = pp;
			fxPos += norm * 48.0;
		}

		//cout << "activating" << endl;

		ActivateEffect(PLAYERFX_LAND_0 + speedLevel, Vector2f(fxPos), RadiansToDegrees(angle), 8, 2, facingRight);
	}

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
	return 10;//30;//1
}

const char * Actor::LAND_GetTilesetName()
{
	return "land_64x64.png";
}