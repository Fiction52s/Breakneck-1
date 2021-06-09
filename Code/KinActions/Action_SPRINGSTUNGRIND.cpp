#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNGRIND_Start()
{
}

void Actor::SPRINGSTUNGRIND_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNGRIND_Change()
{
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	bool j = JumpButtonPressed();

	if (j || touchedGrass[Grass::ANTIGRIND] || springStunFrames == 0 )
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

				if (!j)
				{
					springStunFrames = 0;

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
				}
				else
				{
					springStunFrames = 0;

					ground = grindEdge;
					edgeQuantity = grindQuantity;
					groundSpeed = grindSpeed;
					SetAction(JUMPSQUAT);
					frame = 0;
				}


				grindEdge = NULL;
				reversed = false;
			}

		}
		else if (grindNorm.y > 0)
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
				if (!HasUpgrade(UPGRADE_POWER_GRAV) 
					|| (abs(grindNorm.x) >= wallThresh) 
					|| j || grindEdge->IsInvisibleWall())
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

					springStunFrames = 0;

					SetAction(JUMP);
					frame = 1;
					ground = NULL;
					grindEdge = NULL;
					reversed = false;
				}
				else
				{
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
					reversed = true;
					grindEdge = NULL;


					springStunFrames = 0;

					SetAction(LAND2);
					framesNotGrinding = 0;
					frame = 0;


					double angle = GroundedAngle();

					ActivateEffect(EffectLayer::IN_FRONT, ts_fx_gravReverse, position, false, angle, 25, 1, facingRight);
					ActivateSound(S_GRAVREVERSE);
				}
			}
		}
		else
		{
			if (CheckStandUp())
			{
				springStunFrames = 0;

				framesInAir = 0;
				SetAction(JUMP);
				frame = 1;
				grindEdge = NULL;
				ground = NULL;

				//TODO: this might glitch grind areas? test it with the range of your get out of grind query
				if (grindNorm.x > 0)
				{
					position.x += b.rw + .1;
				}
				else if (grindNorm.x < 0)
				{
					position.x += -b.rw - .1;
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
	/*else if (currInput.rightShoulder && !prevInput.rightShoulder)
	{
		SetAction(GRINDATTACK);
		frame = 0;
	}*/


	/*if (action != GRINDBALL && action != GRINDATTACK)
	{
		hurtBody.isCircle = false;
		hurtBody.rw = 7;
		hurtBody.rh = normalHeight;
	}*/
}

void Actor::SPRINGSTUNGRIND_Update()
{
	if (grindSpeed > 0)
	{
		grindSpeed = std::min(GetMaxSpeed(), grindSpeed);
	}
	else
	{
		grindSpeed = std::max(-GetMaxSpeed(), grindSpeed);
	}

	velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;
}

void Actor::SPRINGSTUNGRIND_UpdateSprite()
{
	//assert(grindEdge != NULL);

	SetSpriteTexture(GRINDBALL);

	V2d grindNorm = grindEdge->Normal();
	bool r = grindSpeed > 0;

	if (IsOnRailAction(action) && grindNorm.y > 0)
	{
		grindNorm = -grindNorm;
		r = !r;
	}

	SetSpriteTile(0, r);

	grindActionCurrent += grindSpeed / 20;
	while (grindActionCurrent >= grindActionLength)
	{
		grindActionCurrent -= grindActionLength;
	}
	while (grindActionCurrent < 0)
	{
		grindActionCurrent += grindActionLength;
	}

	int grindActionInt = grindActionCurrent;

	gsdodeca.setTextureRect(tsgsdodeca->GetSubRect((grindActionInt * 5) % grindActionLength));
	gstriblue.setTextureRect(tsgstriblue->GetSubRect((grindActionInt * 5) % grindActionLength));
	gstricym.setTextureRect(tsgstricym->GetSubRect(grindActionInt % grindActionLength)); //broken?
	gstrigreen.setTextureRect(tsgstrigreen->GetSubRect((grindActionInt * 5) % grindActionLength));
	gstrioran.setTextureRect(tsgstrioran->GetSubRect(grindActionInt% grindActionLength));
	gstripurp.setTextureRect(tsgstripurp->GetSubRect(grindActionInt% grindActionLength));
	gstrirgb.setTextureRect(tsgstrirgb->GetSubRect(grindActionInt% grindActionLength));



	double angle = 0;
	angle = atan2(grindNorm.x, -grindNorm.y);
	if (!approxEquals(abs(offsetX), b.rw))
	{

	}
	else
	{
		//angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
		//angle = asin( dot( grindNorm, V2d( 1, 0 ) ) ); 

	}
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setRotation(angle / PI * 180);
	//	sprite->setRotation( 0 );

	V2d oldv0 = grindEdge->v0;
	V2d oldv1 = grindEdge->v1;

	V2d pp = grindEdge->GetPosition(grindQuantity);

	sprite->setPosition(pp.x, pp.y);


	gsdodeca.setOrigin(gsdodeca.getLocalBounds().width / 2, gsdodeca.getLocalBounds().height / 2);
	gstriblue.setOrigin(gstriblue.getLocalBounds().width / 2, gstriblue.getLocalBounds().height / 2);
	gstricym.setOrigin(gstricym.getLocalBounds().width / 2, gstricym.getLocalBounds().height / 2);
	gstrigreen.setOrigin(gstrigreen.getLocalBounds().width / 2, gstrigreen.getLocalBounds().height / 2);
	gstrioran.setOrigin(gstrioran.getLocalBounds().width / 2, gstrioran.getLocalBounds().height / 2);
	gstripurp.setOrigin(gstripurp.getLocalBounds().width / 2, gstripurp.getLocalBounds().height / 2);
	gstrirgb.setOrigin(gstrirgb.getLocalBounds().width / 2, gstrirgb.getLocalBounds().height / 2);


	gsdodeca.setPosition(pp.x, pp.y);
	gstriblue.setPosition(pp.x, pp.y);
	gstricym.setPosition(pp.x, pp.y);
	gstrigreen.setPosition(pp.x, pp.y);
	gstrioran.setPosition(pp.x, pp.y);
	gstripurp.setPosition(pp.x, pp.y);
	gstrirgb.setPosition(pp.x, pp.y);
}

void Actor::SPRINGSTUNGRIND_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNGRIND_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNGRIND_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNGRIND_GetActionLength()
{
	return 1;
}

Tileset * Actor::SPRINGSTUNGRIND_GetTileset()
{
	return GetActionTileset("grind_64x64.png");
}