#include "Actor.h"
#include "SoundTypes.h"
#include "EditorTerrain.h"

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

	if (j || touchedGrass[Grass::ANTIGRIND] || springStunFrames == 0 || InWater( TerrainPolygon::WATER_INVERTEDINPUTS ) )
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
					reversed = true;
					grindEdge = NULL;


					springStunFrames = 0;

					SetAction(LAND2);
					framesNotGrinding = 0;
					frame = 0;


					double angle = GroundedAngle();

					ActivateEffect(PLAYERFX_GRAV_REVERSE, Vector2f(position), RadiansToDegrees(angle), 25, 1, facingRight);
					ActivateSound(PlayerSounds::S_GRAVREVERSE);
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
	else if (false)//DashButtonPressed() && HasUpgrade(UPGRADE_W5_GRIND_LUNGE)
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

	ActivateLauncherEffect(8);
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

	V2d pp = grindEdge->GetPosition(edgeQuantity);

	sprite->setPosition(pp.x, pp.y);
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

const char * Actor::SPRINGSTUNGRIND_GetTilesetName()
{
	return "grind_64x64.png";
}