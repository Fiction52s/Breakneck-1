#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GRINDATTACK_Start()
{
}

void Actor::GRINDATTACK_End()
{
	SetAction(GRINDBALL);
	frame = 0;
	//SetAction(GRINDATTACK);
	//frame = 0;
}

void Actor::GRINDATTACK_Change()
{
	if (!PowerButtonHeld())//&& grindEdge->Normal().y < 0 )
	{
		V2d op = position;

		V2d grindNorm = grindEdge->Normal();

		if (grindNorm.y < 0)
		{
			double extra = 0;
			if (grindNorm.x > 0)
			{
				offsetX = b.rw;
				extra = .1;
			}
			else if (grindNorm.x < 0)
			{
				offsetX = -b.rw;
				extra = -.1;
			}
			else
			{
				offsetX = 0;
			}

			position.x += offsetX + extra;

			position.y -= normalHeight + .1;

			if (!CheckStandUp())
			{
				position = op;
			}
			else
			{
				if (grindSpeed > 0)
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}

				framesNotGrinding = 0;
				RestoreAirOptions();
				ground = grindEdge;
				edgeQuantity = grindQuantity;
				SetAction(LAND);
				frame = 0;
				groundSpeed = grindSpeed;

				if (currInput.LRight())
				{
					facingRight = true;
					if (groundSpeed < 0)
					{
						groundSpeed = 0;
					}
				}
				else if (currInput.LLeft())
				{
					facingRight = false;
					if (groundSpeed > 0)
					{
						groundSpeed = 0;
					}
				}





				grindEdge = NULL;
				reversed = false;
			}

		}
		else
		{

			if (grindNorm.x > 0)
			{
				position.x += b.rw + .1;
			}
			else if (grindNorm.x < 0)
			{
				position.x += -b.rw - .1;
			}

			if (grindNorm.y > 0)
				position.y += normalHeight + .1;

			if (!CheckStandUp())
			{
				position = op;
			}
			else
			{
				//abs( e0n.x ) < wallThresh )

				if (!HasUpgrade(UPGRADE_POWER_GRAV) || (abs(grindNorm.x) >= wallThresh) || grindEdge->IsInvisibleWall())
				{
					if (grindSpeed < 0)
					{
						facingRight = true;
					}
					else
					{
						facingRight = false;
					}


					framesNotGrinding = 0;
					if (reversed)
					{
						velocity = normalize(grindEdge->v1 - grindEdge->v0) * -grindSpeed;
					}
					else
					{
						velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
					}


					SetAction(JUMP);
					frame = 0;
					ground = NULL;
					grindEdge = NULL;
					reversed = false;
				}
				else
				{
					//	velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
					if (grindNorm.x > 0)
					{
						offsetX = b.rw;
					}
					else if (grindNorm.x < 0)
					{
						offsetX = -b.rw;
					}
					else
					{
						offsetX = 0;
					}

					if (grindSpeed < 0)
					{
						facingRight = true;
					}
					else
					{
						facingRight = false;
					}

					RestoreAirOptions();


					ground = grindEdge;
					groundSpeed = -grindSpeed;
					edgeQuantity = grindQuantity;
					grindEdge = NULL;
					reversed = true;


					if (currInput.LRight())
					{
						if (groundSpeed < 0)
						{
							//cout << "bleh2" << endl;
							groundSpeed = 0;
						}
						facingRight = true;
						//	groundSpeed = abs( groundSpeed );
					}
					else if (currInput.LLeft())
					{
						facingRight = false;
						if (groundSpeed > 0)
						{
							//cout << "bleh1" << endl;
							groundSpeed = 0;
						}
						//	groundSpeed = -abs( groundSpeed );
					}

					SetAction(LAND2);
					frame = 0;
					framesNotGrinding = 0;

					double angle = GroundedAngle();

					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight);
					ActivateSound(S_GRAVREVERSE);
				}
			}
		}
		//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
	}
	else if (DashButtonPressed())
	{
		V2d op = position;

		//V2d op = position;

		V2d grindNorm = grindEdge->Normal();

		if (grindNorm.y < 0)
		{
			double extra = 0;
			if (grindNorm.x > 0)
			{
				offsetX = b.rw;
				extra = .1;
			}
			else if (grindNorm.x < 0)
			{
				offsetX = -b.rw;
				extra = -.1;
			}
			else
			{
				offsetX = 0;
			}

			position.x += offsetX + extra;

			position.y -= normalHeight + .1;

			if (!CheckStandUp())
			{
				position = op;
			}
			else
			{
				SetAction(GRINDLUNGE);
				frame = 0;

				V2d grindNorm = grindEdge->Normal();
				V2d gDir = normalize(grindEdge->v1 - grindEdge->v0);
				lungeNormal = grindNorm;

				double lungeSpeed;
				if (speedLevel == 0)
				{
					lungeSpeed = grindLungeSpeed0;
				}
				else if (speedLevel == 1)
				{
					lungeSpeed = grindLungeSpeed1;
				}
				else if (speedLevel == 2)
				{
					lungeSpeed = grindLungeSpeed2;
				}
				//double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
				//double extra = f * grindLungeExtraMax;

				velocity = lungeNormal * lungeSpeed;//( grindLungeSpeed + extra );

													/*double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
													double extra = f * grindLungeExtraMax;

													velocity = lungeNormal * ( grindLungeSpeed + extra );*/

													/*if( currInput.A )
													{
													velocity += gDir * grindSpeed;
													V2d normV = normalize( velocity );
													lungeNormal = V2d( normV.y, -normV.x );
													}*/

													//grindEdge = NULL;

				facingRight = (grindNorm.x > 0);

				grindEdge = NULL;
				ground = NULL;
			}

		}
		else
		{

			if (grindNorm.x > 0)
			{
				position.x += b.rw + .1;
			}
			else if (grindNorm.x < 0)
			{
				position.x += -b.rw - .1;
			}

			if (grindNorm.y > 0)
				position.y += normalHeight + .1;

			if (!CheckStandUp())
			{
				position = op;
			}
			else
			{
				SetAction(GRINDLUNGE);
				frame = 0;

				V2d grindNorm = grindEdge->Normal();
				V2d gDir = normalize(grindEdge->v1 - grindEdge->v0);

				lungeNormal = grindNorm;
				double lungeSpeed;
				if (speedLevel == 0)
				{
					lungeSpeed = grindLungeSpeed0;
				}
				else if (speedLevel == 1)
				{
					lungeSpeed = grindLungeSpeed1;
				}
				else if (speedLevel == 2)
				{
					lungeSpeed = grindLungeSpeed2;
				}
				//double f = max( abs( grindSpeed ) - 20.0, 0.0 ) / maxGroundSpeed;
				//double extra = f * grindLungeExtraMax;

				velocity = lungeNormal * lungeSpeed;//( grindLungeSpeed + extra );

				facingRight = (grindNorm.x > 0);

				grindEdge = NULL;
				ground = NULL;
			}
		}
		//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
	}
	else if (!currInput.rightShoulder)
	{
		SetAction(GRINDBALL);
		frame = 0;
	}

	if (action != GRINDBALL && action != GRINDATTACK)
	{
		hurtBody.isCircle = false;
		hurtBody.rw = 7;
		hurtBody.rh = normalHeight;
	}
}

void Actor::GRINDATTACK_Update()
{
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	/*double dSpeed = GetDashSpeed();
	double adSpeed = abs(dSpeed);
	double tot = (abs(grindSpeed) - adSpeed) / (maxGroundSpeed - adSpeed);

	double fac = .2;

	if (tot > .5)
		fac = .5;

	if (grindSpeed > dSpeed)
	{
		grindSpeed -= fac * slowMultiple;
		if (grindSpeed < dSpeed)
			grindSpeed = dSpeed;
	}
	else if (grindSpeed < -dSpeed)
	{
		grindSpeed += fac * slowMultiple;
		if (grindSpeed > -dSpeed)
			grindSpeed = -dSpeed;
	}*/


	velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;

	/*if (abs(grindSpeed) > dSpeed)
	{
		SetCurrHitboxes(grindHitboxes[0], 0);
	}*/
	SetCurrHitboxes(grindHitboxes[0], 0);
}

void Actor::GRINDATTACK_UpdateSprite()
{
	GRINDBALL_UpdateSprite();

	V2d grindNorm = grindEdge->Normal();
	bool r = grindSpeed > 0;

	if (IsOnRailAction(action) && grindNorm.y > 0)
	{
		grindNorm = -grindNorm;
		r = !r;
	}

	ts_grindAttackFX->SetSubRect(grindAttackSprite, frame, r);
	grindAttackSprite.setPosition(position.x, position.y);
	grindAttackSprite.setOrigin(grindAttackSprite.getLocalBounds().width / 2,
		grindAttackSprite.getLocalBounds().height / 2);

}

void Actor::GRINDATTACK_TransitionToAction(int a)
{

}

void Actor::GRINDATTACK_TimeIndFrameInc()
{

}

void Actor::GRINDATTACK_TimeDepFrameInc()
{

}

int Actor::GRINDATTACK_GetActionLength()
{
	return 20;
}

Tileset * Actor::GRINDATTACK_GetTileset()
{
	return GetActionTileset("grind_64x64.png");
}