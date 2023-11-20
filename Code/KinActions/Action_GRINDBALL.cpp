#include "Actor.h"
#include "VisualEffects.h"
#include "EditorTerrain.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::GRINDBALL_Start()
{
	distanceGrinded = 0;
}

void Actor::GRINDBALL_End()
{
	frame = 0;
}

void Actor::GRINDBALL_Change()
{
	framesSinceGrindAttempt = maxFramesSinceGrindAttempt;
	bool j = JumpButtonPressed();
	//bool isntWall = grindEdge->Normal().y != 0;

	bool exitedGrind = false;

	if ((framesGrinding > 0 && !GrindButtonHeld()) || j || touchedGrass[Grass::ANTIGRIND] || InWater(TerrainPolygon::WATER_FREEFLIGHT)
		|| InWater(TerrainPolygon::WATER_INVERTEDINPUTS) || InWater(TerrainPolygon::WATER_GLIDE))
	{
		if (ExitGrind(j))
		{
			exitedGrind = true;
		}
	}

	if (!exitedGrind)
	{
		if (DashButtonPressed() && HasUpgrade(UPGRADE_W5_GRIND_LUNGE))
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
		else if (currInput.rightShoulder && !prevInput.rightShoulder)
		{
			SetAction(GRINDATTACK);
			frame = 0;
		}
	}


	if (action != GRINDBALL && action != GRINDATTACK)
	{
		hurtBody.isCircle = false;
		hurtBody.rw = 7;
		hurtBody.rh = normalHeight;

		framesNotGrinding = 0;
	}
}

void Actor::GRINDBALL_Update()
{
	double decel = .2;
	double slowDecel = 0;
	
	double dSpeed = GetDashSpeed();
	double grindDecelLimit = GetMaxSpeed() / 2.0;//30; //might need to adjust more later

	double currDecel = slowDecel;
	if (framesGrinding >= grindLimitBeforeSlow)
	{
		currDecel = decel;
	}

	if (grindSpeed > 0)
	{
		grindSpeed = std::min(GetMaxSpeed(), grindSpeed);

		double oldGrindSpeed = grindSpeed;


		if (grindSpeed > grindDecelLimit)
		{
			grindSpeed -= currDecel;

			if (grindSpeed < grindDecelLimit)
			{
				grindSpeed = grindDecelLimit;
			}
		}
	}
	else
	{
		grindSpeed = std::max(-GetMaxSpeed(), grindSpeed);

		double oldGrindSpeed = grindSpeed;

		if (grindSpeed < -grindDecelLimit)
		{
			grindSpeed += currDecel;

			if (grindSpeed > -grindDecelLimit)
			{
				grindSpeed = -grindDecelLimit;
			}
		}
	}

	velocity = normalize(grindEdge->v1 - grindEdge->v0) * grindSpeed;

	distanceGrinded += grindSpeed / slowMultiple;
}

void Actor::GRINDBALL_UpdateSprite()
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

	V2d grindPoint = grindEdge->GetPosition(edgeQuantity);
	V2d pp = grindPoint + grindNorm * 20.0;

	sprite->setPosition(pp.x, pp.y);

	double gAng = distanceGrinded /50.0;
	double realGAng = gAng;

	for (int i = 0; i < NUM_GRIND_QUADS; ++i)
	{
		realGAng = gAng;


		if (i == 1)
		{
			realGAng *= .8;
		}
		else if (i == 2)
		{
			realGAng *= 1.3;
			realGAng += PI;
		}
		else if (i == 3)
		{
			realGAng *= 1.7;
			realGAng += PI;
		}

		
		SetRectRotation(grindQuads + i * 4, realGAng, ts_grind->tileWidth, ts_grind->tileHeight, Vector2f(grindPoint));

		ts_grind->SetQuadSubRect(grindQuads + i * 4, i);
		
	}

	

	if (framesGrinding % 10 == 0 && framesGrinding >= grindLimitBeforeSlow)
	{
		RelEffectInstance params;
		//EffectInstance params;
		Transform tr = sf::Transform::Identity;

		int dist = 80;
		//params.SetParams(Vector2f(position.x, position.y - 100) , tr, 7, 1, 0);
		Vector2f randPos(rand() % dist - dist / 2, rand() % dist - dist / 2);

		//params.SetParams(randPos, tr, 24, 1, 0, &spriteCenter);
		params.SetParams(randPos, tr, 8, 1, 0, &spriteCenter);

		ActivateEffect(PLAYERFX_SPRINT_STAR, &params);
	}
}

void Actor::GRINDBALL_TransitionToAction(int a)
{

}

void Actor::GRINDBALL_TimeIndFrameInc()
{

}

void Actor::GRINDBALL_TimeDepFrameInc()
{

}

int Actor::GRINDBALL_GetActionLength()
{
	return 1;
}

const char * Actor::GRINDBALL_GetTilesetName()
{
	return "grind_64x64.png";
}