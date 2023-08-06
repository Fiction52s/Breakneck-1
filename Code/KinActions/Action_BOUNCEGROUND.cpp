#include "Actor.h"
#include <iostream>
#include "Session.h"
#include "SoundTypes.h"
#include "EditorTerrain.h"

using namespace std;
using namespace sf;


void Actor::BOUNCEGROUND_Start()
{

}

void Actor::BOUNCEGROUND_End()
{
	frame = 0;
}

void Actor::BOUNCEGROUND_Change()
{
	//if (TryGroundBlock()) return true;

	//if (TryFloorRailDropThrough()) return true;

	//if (TryPressGrind()) return true;

	//if (TryJumpSquat()) return true;

	//if (TryGroundAttack()) return true;

	//if (TryDash()) return true;

	////control only

	//if (BasicSteepAction(gNorm)) return true;

	//if (TrySprintOrRun(gNorm)) return true;

	//if (TrySlideBrakeOrStand()) return true;


	//if (!PowerButtonHeld() || currPowerMode != PMODE_BOUNCE )
	if( !bounceFlameOn)
	{
		SetAction(JUMP);
		velocity = storedBounceVel;
		frame = 1;
		BounceFlameOff();
		holdJump = false;
		return;
	}

	//if (TryGroundBlock()) return true;

	//if (TryFloorRailDropThrough()) return true;

	//if (TryPressGrind()) return true;
	if (JumpButtonPressed())
	{
		if (TryLandFromBounceGround())
		{
			SetAction(JUMPSQUAT);
			frame = 0;
			return;
		}
		else
		{
			if (TryDoubleJump())
			{
				bounceEdge = NULL;
				velocity = storedBounceVel;
				holdJump = false;
				return;
			}
		}
		
	}
	
	/*if (AttackButtonPressed())
	{
		if (TryLandFromBounceGround())
		{
			if (TryGroundAttack())
			{
				return;
			}
			else
			{
				assert(0);
			}
		}
		else
		{
			if (AirAttack())
			{
				bounceEdge = NULL;
				velocity = storedBounceVel;
				holdJump = false;
				return;
			}
			else
			{
				assert(0);
			}
		}
	}*/
	
	if (DashButtonPressed())
	{
		if (bounceNorm.y < 0 && TerrainPolygon::IsSteepGround(bounceNorm))
		{
			if ((bounceNorm.x < 0 && (currInput.LRight() || currInput.LUp()) && !currInput.LLeft() && !currInput.LDown())
				|| (bounceNorm.x > 0 && (currInput.LLeft() || currInput.LUp()) && !currInput.LRight() && !currInput.LDown()))
			{
				if (!TryLandFromBounceGround())
				{
					assert(0);
				}
				SetAction(STEEPCLIMB);
				frame = 0;
				return;
			}
		}
		else if (TryLandFromBounceGround())
		{
			SetAction(DASH);
			frame = 0;
			return;
		}
		else
		{
			if (TryAirDash())
			{
				bounceEdge = NULL;
				velocity = storedBounceVel;
				holdJump = false;
				return;
			}
		}
		
	}

	/*if (TryJumpSquat())
	{
		
		return;
	}
	

	if (TryGroundAttack()) return;

	if (TryDash()) return;*/


	V2d bn = bounceNorm;//bounceEdge->Normal();
	bool framesDone = frame == GetActionLength(BOUNCEGROUND) - 1;
	if (boostBounce || (framesDone && bn.y >= 0))
	{
		ActivateSound(PlayerSounds::S_BOUNCEJUMP);
		framesInAir = 0;
		SetAction(BOUNCEAIR);
		oldBounceEdge = bounceEdge;
		oldBounceNorm = bounceNorm;
		frame = 0;

		double currBoostBounceSpeed = GetBounceBoostSpeed();

		int option = 0; //0 is ground, 1 is wall, 2 is ceiling

		bool boostNow = boostBounce && framesSinceBounce > 8;

		double extraBUp = .2;
		double extraBDown = .2;
		double extraBThresh = .8; //works on almost everything
		double dSpeed = GetDashSpeed();

		V2d oldVel = velocity;

		//velocity = bounceEdge->Normal() * 20.0;

		

		velocity = sess->CalcBounceReflectionVel(bn, storedBounceVel);

		
		V2d vDir = normalize(velocity);

		if (bn.y != 0)
		{
			if ((bn.x > 0 && velocity.x > 0) || (bn.x < 0 && velocity.x < 0))
			{
				velocity = bn * length(velocity);
			}
		}
		else
		{
			//dj refresh just like walljump, can turn into a function later
			if (hasWallJumpRechargeAirDash && !hasAirDash)
			{
				hasAirDash = true;

				if (HasUpgrade(UPGRADE_W7_DOUBLE_AIRDASH_BOOST))
				{
					numRemainingExtraAirdashBoosts = 1;
				}

				hasWallJumpRechargeAirDash = false;
			}
			if (hasWallJumpRechargeDoubleJump && !hasDoubleJump)
			{
				hasDoubleJump = true;
				hasWallJumpRechargeDoubleJump = false;
			}


			if (currInput.LUp())
			{
				vDir = normalize(vDir + V2d(0, -extraBUp));
				velocity = vDir * length(velocity);
			}
			else if (currInput.LDown())
			{
				vDir = normalize(vDir + V2d(0, extraBDown));
				velocity = vDir * length(velocity);
			}
		}
		

		velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;

		if (bounceNorm.y != 0)
		{
			if (oldVel.x > 0 && bounceNorm.x > 0 && velocity.x < oldVel.x)
			{
				velocity.x = oldVel.x;
			}
			else if (oldVel.x < 0 && bounceNorm.x < 0 && velocity.x < oldVel.x)
			{
				velocity.x = oldVel.x;
			}
		}
		

		//minimum bounce along normal. wall minimum bounce is smaller
		double alongNormal = dot(velocity, bn);
		double limit = 10.0;
		if (bn.x == 1.0 || bn.x == -1.0)
		{
			limit = 2.0;
		}
		if ( alongNormal < limit)
		{
			//cout << "heres the test" << endl;
			double diff = limit - alongNormal;
			velocity += diff * bn;
		}

		if (bn.y == 1 || bn.y == -1)
		{
			if (velocity.x == 0)
			{
				if (currInput.LLeft())
				{
					velocity.x = -dSpeed;
				}
				else if (currInput.LRight())
				{
					velocity.x = dSpeed;
				}
			}
			else if (velocity.x > 0)
			{
				if (currInput.LLeft())
				{
					velocity.x = -dSpeed;
				}
			}
			else if (velocity.x < 0)
			{
				if (currInput.LRight())
				{
					velocity.x = dSpeed;
				}
			}
		}


		//double currSpeed = length(velocity);

		//if (bn.y < 0)
		//{
		//	double testVel = 25;
		//	if (velocity.y > -testVel)
		//		velocity.y = -testVel;

		//	vDir = normalize(velocity);

		//	velocity = vDir * currSpeed;

		//	velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;

		//	//velocity.y = -ySpeed;
		//}







		/*velocity = sess->CalcBounceReflectionVel(bounceEdge, storedBounceVel);
		


		//if (bn.y < 0)
		//{
		//	//cout << "prevel: " << velocity.x << ", " << velocity.y << endl;
		//	if (bn.y > -steepThresh)
		//	{
		//		//if( bn.x > 0  && storedBounceVel.x < 0 )
		//		//{
		//		//	//cout << "A" << endl;
		//		//	velocity = V2d( abs(storedBounceVel.x), -abs(storedBounceVel.y) );
		//		//}
		//		//else if( bn.x < 0 && storedBounceVel.x > 0 )
		//		//{
		//		////	cout << "B" << endl;
		//		//	velocity = V2d( -abs(storedBounceVel.x), -abs(storedBounceVel.y) );
		//		//}
		//		//else
		//		{
		//			velocity = sess->CalcBounceReflectionVel(bounceEdge, storedBounceVel);
		//		}

		//		double velStrength = length(velocity);
		//		V2d vDir = normalize(velocity);
		//		if (abs(vDir.y) < extraBThresh)
		//		{
		//			if (currInput.LUp())
		//			{
		//				vDir = normalize(vDir + V2d(0, -extraBUp));
		//			}
		//			else if (currInput.LDown())
		//			{
		//				vDir = normalize(vDir + V2d(0, extraBDown));
		//			}
		//			//velocity = vDir * velStrength;
		//		}

		//		if (boostNow)
		//		{
		//			//double fac = max( 6.0, .3 * velocity.y ); //bounceBoostSpeed;
		//			velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;


		//			boostBounce = false;
		//		}
		//		else if (boostBounce)
		//		{
		//			boostBounce = false;
		//		}
		//	}
		//	else
		//	{
		//		if (storedBounceVel.x > 0 && currInput.LLeft())
		//		{
		//			storedBounceVel.x = -dSpeed;
		//		}
		//		else if (storedBounceVel.x < 0 && currInput.LRight())
		//		{
		//			storedBounceVel.x = dSpeed;
		//		}
		//		else if (storedBounceVel.x == 0)
		//		{
		//			if (currInput.LLeft())
		//			{
		//				storedBounceVel.x = -maxAirXControl;
		//			}
		//			else if (currInput.LRight())
		//			{
		//				storedBounceVel.x = maxAirXControl;
		//			}
		//		}

		//		double bouncePower = 20;
		//		double absY = abs(storedBounceVel.y);
		//		bouncePower = max(bouncePower, absY);
		//		

		//		velocity = V2d(storedBounceVel.x, -bouncePower);//length( storedBounceVel ) * bounceEdge->Normal();

		//		if (boostNow)
		//		{
		//			//6.0
		//			double fac = max(currBoostBounceSpeed, .25 * abs(velocity.y));
		//														  
		//			velocity += normalize(velocity) * fac / (double)slowMultiple;

		//			/*double shardSpeedBoost = 5;
		//			if (velocity.x > 0)
		//			{
		//				velocity.x += shardSpeedBoost;
		//			}
		//			else if (velocity.x < 0)
		//			{
		//				velocity.x -= shardSpeedBoost;
		//			}*/
		//			boostBounce = false;
		//		}
		//		else if (boostBounce)
		//		{
		//			boostBounce = false;
		//		}

		//	}
		//}
		//else if (bn.y > 0)
		//{
		//	if (-bn.y > -steepThresh)
		//	{
		//		//if( bn.x > 0 && storedBounceVel.x < 0 )
		//		//{
		//		////	cout << "C" << endl;
		//		//	velocity = V2d( abs(storedBounceVel.x), storedBounceVel.y );
		//		//}
		//		//else if( bn.x < 0 && storedBounceVel.x > 0 )
		//		//{
		//		////	cout << "D" << endl;
		//		//	velocity = V2d( -abs(storedBounceVel.x), storedBounceVel.y );
		//		//}
		//		//else
		//		{
		//			double lenVel = length(storedBounceVel);
		//			double reflX = cross(normalize(-storedBounceVel), bn);
		//			double reflY = dot(normalize(-storedBounceVel), bn);
		//			V2d edgeDir = normalize(bounceEdge->v1 - bounceEdge->v0);
		//			velocity = normalize(reflX * edgeDir + reflY * bn) * lenVel;
		//		}
		//	}
		//	else
		//	{
		//		if (storedBounceVel.x == 0)
		//		{
		//			if (currInput.LLeft())
		//			{
		//				storedBounceVel.x = -maxAirXControl;
		//			}
		//			else if (currInput.LRight())
		//			{
		//				storedBounceVel.x = maxAirXControl;
		//			}
		//		}


		//		velocity = V2d(storedBounceVel.x, abs(storedBounceVel.y));//length( storedBounceVel ) * bounceEdge->Normal();
		//																  //	cout << "E: " << velocity.x << ", " << velocity.y << endl;

		//	}

		//	double velStrength = length(velocity);
		//	V2d vDir = normalize(velocity);
		//	if (abs(vDir.y) < extraBThresh)
		//	{
		//		if (currInput.LUp())
		//		{
		//			vDir = normalize(vDir + V2d(0, -extraBUp));
		//		}
		//		else if (currInput.LDown())
		//		{
		//			vDir = normalize(vDir + V2d(0, extraBDown));
		//		}
		//		//velocity = vDir * velStrength;
		//	}

		//	if (boostNow)
		//	{
		//		//double fac = max( 6.0, .3 * velocity.y ); //bounceBoostSpeed;
		//		//velocity += normalize( velocity ) * bounceBoostSpeed / (double)slowMultiple;
		//		velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;

		//		boostBounce = false;
		//	}
		//	else if (boostBounce)
		//	{
		//		boostBounce = false;
		//	}
		//}
		//else
		//{
		//	//	cout << "F" << endl;
		//	velocity = V2d(-storedBounceVel.x, storedBounceVel.y);

		//	double velStrength = length(velocity);
		//	V2d vDir = normalize(velocity);
		//	if (abs(vDir.y) < extraBThresh)
		//	{
		//		if (currInput.LUp())
		//		{
		//			vDir = normalize(vDir + V2d(0, -extraBUp));
		//		}
		//		else if (currInput.LDown())
		//		{
		//			vDir = normalize(vDir + V2d(0, extraBDown));
		//		}
		//		velocity = vDir * velStrength;
		//	}

		//	if (boostNow)
		//	{
		//		velocity += vDir * currBoostBounceSpeed / (double)slowMultiple;
		//		/*if( currInput.LUp() )
		//		{
		//		velocity += V2d( 0, -1 ) * extraBUp;
		//		}
		//		else if( currInput.LDown() )
		//		{
		//		velocity += V2d( 0, 1 ) * extraBDown;
		//		}*/

		//		boostBounce = false;
		//	}
		//	else if (boostBounce)
		//	{
		//		boostBounce = false;
		//	}
		//}

		//velocity += V2d( 0, -gravity * slowMultiple );
		if (facingRight && velocity.x < 0)
			facingRight = false;
		else if (!facingRight && velocity.x > 0)
			facingRight = true;

		/*double lenVel = length( storedBounceVel );
		double reflX = cross( normalize( -storedBounceVel ), bn );
		double reflY = dot( normalize( -storedBounceVel ), bn );
		V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
		velocity = normalize( reflX * edgeDir + reflY * bn ) * lenVel;*/


		framesSinceBounce = 0;

		//velocity = length( storedBounceVel ) * bounceEdge->Normal();
		//ground = NULL;
		bounceEdge = NULL;

		//if( ground != NULL )
		//	ground = NULL;
	}
	else if (framesDone)
	{
		if (bn.y < 0)
		{
			V2d alongVel = V2d(-bn.y, bn.x);
			ground = bounceEdge;
			bounceEdge = NULL;

			if (bn.y > -steepThresh)
			{

			}
			else
			{
			}
			SetAction(LAND);
			frame = 0;

			bounceFlameOn = true;
			scorpOn = true;

			V2d testVel = storedBounceVel;


			if (testVel.y > 20)
			{
				testVel.y *= .7;
			}
			else if (testVel.y < -30)
			{

				testVel.y *= .5;
			}

			groundSpeed = CalcLandingSpeed(testVel, alongVel, bn);


			currNormal = ground->Normal();

			//if( gNorm.y <= -steepThresh )
			{
				RestoreAirOptions();
			}

			if (testVel.x < 0 && currNormal.y <= -steepThresh)
			{
				groundSpeed = min(testVel.x, dot(testVel, normalize(ground->v1 - ground->v0)) * .7);
				//cout << "left boost: " << groundSpeed << endl;
			}
			else if (testVel.x > 0 && currNormal.y <= -steepThresh)
			{
				groundSpeed = max(testVel.x, dot(testVel, normalize(ground->v1 - ground->v0)) * .7);
				//cout << "right boost: " << groundSpeed << endl;
			}


		}
		else
		{
			SetAction(JUMP);
			frame = 1;
			velocity = storedBounceVel;
			bounceEdge = NULL;
		}
	}
}

void Actor::BOUNCEGROUND_Update()
{
	if (!boostBounce && frame == 4 )//&& JumpButtonPressed() )
	{
		//ActivateSound( soundBuffers[S_BOUNCEJUMP] );
		boostBounce = true;


		V2d bouncePos = bounceEdge->GetPosition(edgeQuantity);
		V2d bn = bounceEdge->Normal();
		double angle = atan2(bn.x, -bn.y);
		bouncePos += bn * 80.0;
		ActivateEffect(PLAYERFX_BOUNCE_BOOST, Vector2f(bouncePos), RadiansToDegrees(angle), 30, 1, facingRight);
	}

	velocity.x = 0;
	velocity.y = 0;
	groundSpeed = 0;
}

void Actor::BOUNCEGROUND_UpdateSprite()
{
	int bounceFrame = 0;
	V2d bn = bounceNorm;//bounceEdge->Normal();

	bool bounceFacingRight = facingRight;

	if (bn.y <= 0 && bn.y > -steepThresh)
	{
		//if( storedBounceVel.y > 0 ) //falling
		//{

		//}
		//else
		//{

		//}
		//if( (bn.x > 0 && storedBounceVel.x >= 0) || (bn.x < 0 && storedBounceVel.x <= 0 ) )
		//	bounceFrame = 0;
		//else
		//	bounceFrame = 2;

		bounceFrame = 2;

		bounceFacingRight = (bn.x > 0);
	}
	else if (bn.y >= 0 && -bn.y > -steepThresh)
	{
		/*if( (bn.x > 0 && storedBounceVel.x >= 0) || (bn.x < 0 && storedBounceVel.x <= 0 ) )
		bounceFrame = 4;
		else
		bounceFrame = 2;*/
		bounceFrame = 2;
		bounceFacingRight = (bn.x > 0);
		//facingRight = !facingRight;
	}
	else if (bn.y == 0)
	{
		bounceFrame = 2;
		//	facingRight = !facingRight;
	}
	else if (bn.y < 0)
	{
		bounceFrame = 0;//8
	}
	else if (bn.y > 0)
	{
		bounceFrame = 4;
	}


	SetSpriteTexture(action);

	//bool r = (bounceFacingRight && !reversed ) || (!bounceFacingRight && reversed );
	SetSpriteTile(bounceFrame, bounceFacingRight);

	double angle = 0;
	if (!approxEquals(abs(offsetX), b.rw))
	{
		if (reversed)
			angle = PI;
	}
	else
	{
		angle = atan2(bn.x, -bn.y);
	}

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setRotation(0);

	assert(scorpOn);
	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpBounce->texture);
		SetSpriteTile(&scorpSprite, ts_scorpBounce, bounceFrame, facingRight);

		/*if (r)
		{
		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 + 30,
		scorpSprite.getLocalBounds().height / 2 + 25);
		}
		else
		{
		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2 - 30,
		scorpSprite.getLocalBounds().height / 2 + 25);
		}*/

		scorpSprite.setPosition(position.x, position.y);

		if (bounceFrame == 4)
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
				scorpSprite.getLocalBounds().height / 2 - 60);
		}
		else
		{
			scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
				scorpSprite.getLocalBounds().height / 2);
		}

		
		scorpSprite.setRotation(0);//sprite->getRotation());
		scorpSet = true;
	}
	//if (abs(bn.x) >= wallThresh)
	//{
	//	if (bn.x > 0)
	//	{
	//		sprite->setOrigin(110, sprite->getLocalBounds().height / 2);
	//	}
	//	else
	//	{
	//		sprite->setOrigin(sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
	//	}
	//}
	//else if (bn.y <= 0 && bn.y > -steepThresh)
	//{
	//	if (bounceFacingRight)
	//	{
	//		sprite->setOrigin(110, sprite->getLocalBounds().height / 2);
	//		//sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
	//	}
	//	else
	//	{
	//		//sprite->setOrigin( , sprite->getLocalBounds().height / 2);
	//		sprite->setOrigin(sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
	//	}
	//}
	//else if (bn.y >= 0 && -bn.y > -steepThresh)
	//{
	//	if (bounceFacingRight)//bounceFrame == 4 )
	//	{
	//		sprite->setOrigin(110, sprite->getLocalBounds().height / 2);
	//		//sprite->setOrigin( sprite->getLocalBounds().width / 2, 0);
	//		//sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
	//	}
	//	else
	//	{
	//		sprite->setOrigin(sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
	//		//sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
	//	}
	//}
	//else if (bn.y < 0)
	//{
	//	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - 120);
	//}
	//else if (bn.y > 0)
	//{
	//	//cout << "this one" << endl;
	//	sprite->setOrigin(sprite->getLocalBounds().width / 2, 80);
	//}
	if (bounceFrame == 4) //ceiling
	{
		sprite->setOrigin(sprite->getLocalBounds().width / 2, 22 );
	}

	sprite->setPosition(position.x, position.y);

	scorpSet = true;
}

void Actor::BOUNCEGROUND_TransitionToAction(int a)
{

}

void Actor::BOUNCEGROUND_TimeIndFrameInc()
{

}

void Actor::BOUNCEGROUND_TimeDepFrameInc()
{

}

int Actor::BOUNCEGROUND_GetActionLength()
{
	return 15;
}

Tileset * Actor::BOUNCEGROUND_GetTileset()
{
	return GetActionTileset("bounce_96x96.png");
}