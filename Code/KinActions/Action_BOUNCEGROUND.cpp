#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BOUNCEGROUND_Start()
{

}

void Actor::BOUNCEGROUND_End()
{
	frame = 0;
}

void Actor::BOUNCEGROUND_Change()
{
	if (!currInput.X)
	{


		SetAction(JUMP);
		velocity = storedBounceVel;
		frame = 1;
		BounceFlameOff();
		holdJump = false;
		return;
	}



	V2d bn = bounceNorm;//bounceEdge->Normal();
	bool framesDone = frame == GetActionLength(BOUNCEGROUND) - 1;
	if (boostBounce || (framesDone && bn.y >= 0))
	{
		ActivateSound(S_BOUNCEJUMP);
		framesInAir = 0;
		SetAction(BOUNCEAIR);
		oldBounceEdge = bounceEdge;
		oldBounceNorm = bounceNorm;
		frame = 0;


		int option = 0; //0 is ground, 1 is wall, 2 is ceiling

		bool boostNow = boostBounce && framesSinceBounce > 8;

		//double lenVel = length( storedBounceVel );
		//double reflX = cross( normalize( -storedBounceVel ), bn );
		//double reflY = dot( normalize( -storedBounceVel ), bn );
		//V2d edgeDir = normalize( bounceEdge->v1 - bounceEdge->v0 );
		//V2d ref = normalize( reflX * edgeDir + reflY * bn ) * lenVel;

		//double c = cos( -currInput.leftStickRadians );
		//double s = sin( -currInput.leftStickRadians );
		//V2d left( c, s );

		//double dd = dot( ref, left );
		//double cc = cross( ref, left );

		////V2d ne( ref.x * c + ref.y * -s, ref.x * s + ref.y * c );
		//V2d eft = left + V2d( 0, -1 );
		//eft /= 2.0;
		//velocity = left * lenVel;





		//dot( ref, eft );//dd * edgeDir + cc * bn;
		//cout << "setting vel: " << velocity.x << ", " << velocity.y << endl;
		//double dd = dot( 
		//velocity = normalize( 
		double extraBUp = .2;
		double extraBDown = .2;
		double extraBThresh = .8; //works on almost everything
		double dSpeed = GetDashSpeed();
		if (bn.y < 0)
		{
			//cout << "prevel: " << velocity.x << ", " << velocity.y << endl;
			if (bn.y > -steepThresh)
			{
				//if( bn.x > 0  && storedBounceVel.x < 0 )
				//{
				//	//cout << "A" << endl;
				//	velocity = V2d( abs(storedBounceVel.x), -abs(storedBounceVel.y) );
				//}
				//else if( bn.x < 0 && storedBounceVel.x > 0 )
				//{
				////	cout << "B" << endl;
				//	velocity = V2d( -abs(storedBounceVel.x), -abs(storedBounceVel.y) );
				//}
				//else
				{
					double lenVel = length(storedBounceVel);
					double reflX = cross(normalize(-storedBounceVel), bn);
					double reflY = dot(normalize(-storedBounceVel), bn);
					V2d edgeDir = normalize(bounceEdge->v1 - bounceEdge->v0);
					//velocity = V2d( abs(storedBounceVel.x), -abs(storedBounceVel.y) );
					//cout << "reflx: " << reflX <<", refly: " << reflY << endl;
					velocity = normalize(reflX * edgeDir + reflY * bn) * lenVel;
					//cout << "set vel: " << velocity.x << ", " << velocity.y << endl;
				}

				double velStrength = length(velocity);
				V2d vDir = normalize(velocity);
				if (abs(vDir.y) < extraBThresh)
				{
					if (currInput.LUp())
					{
						vDir = normalize(vDir + V2d(0, -extraBUp));
					}
					else if (currInput.LDown())
					{
						vDir = normalize(vDir + V2d(0, extraBDown));
					}
					//velocity = vDir * velStrength;
				}

				if (boostNow)
				{
					//double fac = max( 6.0, .3 * velocity.y ); //bounceBoostSpeed;
					velocity += vDir * bounceBoostSpeed / (double)slowMultiple;


					boostBounce = false;
				}
				else if (boostBounce)
				{
					boostBounce = false;
				}
				//if( 
				//bounceNorm.y = -1;
			}
			else
			{
				//cout << "DD" << endl;


				if (storedBounceVel.x > 0 && currInput.LLeft())
				{
					storedBounceVel.x = -dSpeed;
				}
				else if (storedBounceVel.x < 0 && currInput.LRight())
				{
					storedBounceVel.x = dSpeed;
				}
				else if (storedBounceVel.x == 0)
				{
					if (currInput.LLeft())
					{
						storedBounceVel.x = -maxAirXControl;
					}
					else if (currInput.LRight())
					{
						storedBounceVel.x = maxAirXControl;
					}
				}


				velocity = V2d(storedBounceVel.x, -abs(storedBounceVel.y));//length( storedBounceVel ) * bounceEdge->Normal();

				if (boostNow)
				{
					double fac = max(6.0, .25 * abs(velocity.y)); //bounceBoostSpeed;
																  //cout << "fac: " << fac << ", vy: "<< velocity.y << endl;
					velocity += normalize(velocity) * fac / (double)slowMultiple;
					boostBounce = false;
				}
				else if (boostBounce)
				{
					boostBounce = false;
				}

			}
		}
		else if (bn.y > 0)
		{
			if (-bn.y > -steepThresh)
			{
				//if( bn.x > 0 && storedBounceVel.x < 0 )
				//{
				////	cout << "C" << endl;
				//	velocity = V2d( abs(storedBounceVel.x), storedBounceVel.y );
				//}
				//else if( bn.x < 0 && storedBounceVel.x > 0 )
				//{
				////	cout << "D" << endl;
				//	velocity = V2d( -abs(storedBounceVel.x), storedBounceVel.y );
				//}
				//else
				{
					double lenVel = length(storedBounceVel);
					double reflX = cross(normalize(-storedBounceVel), bn);
					double reflY = dot(normalize(-storedBounceVel), bn);
					V2d edgeDir = normalize(bounceEdge->v1 - bounceEdge->v0);
					velocity = normalize(reflX * edgeDir + reflY * bn) * lenVel;
				}

				//if( 
				//bounceNorm.y = -1;
			}
			else
			{
				/*if( storedBounceVel.x > 0 && currInput.LLeft() )
				{
				storedBounceVel.x = -dashSpeed;
				}
				else if( storedBounceVel.x < 0 && currInput.LRight() )
				{
				storedBounceVel.x = dashSpeed;
				}
				else */
				if (storedBounceVel.x == 0)
				{
					if (currInput.LLeft())
					{
						storedBounceVel.x = -maxAirXControl;
					}
					else if (currInput.LRight())
					{
						storedBounceVel.x = maxAirXControl;
					}
				}


				velocity = V2d(storedBounceVel.x, abs(storedBounceVel.y));//length( storedBounceVel ) * bounceEdge->Normal();
																		  //	cout << "E: " << velocity.x << ", " << velocity.y << endl;

			}

			double velStrength = length(velocity);
			V2d vDir = normalize(velocity);
			if (abs(vDir.y) < extraBThresh)
			{
				if (currInput.LUp())
				{
					vDir = normalize(vDir + V2d(0, -extraBUp));
				}
				else if (currInput.LDown())
				{
					vDir = normalize(vDir + V2d(0, extraBDown));
				}
				//velocity = vDir * velStrength;
			}

			if (boostNow)
			{
				//double fac = max( 6.0, .3 * velocity.y ); //bounceBoostSpeed;
				//velocity += normalize( velocity ) * bounceBoostSpeed / (double)slowMultiple;
				velocity += vDir * bounceBoostSpeed / (double)slowMultiple;

				boostBounce = false;
			}
			else if (boostBounce)
			{
				boostBounce = false;
			}
		}
		else
		{
			//	cout << "F" << endl;
			velocity = V2d(-storedBounceVel.x, storedBounceVel.y);

			double velStrength = length(velocity);
			V2d vDir = normalize(velocity);
			if (abs(vDir.y) < extraBThresh)
			{
				if (currInput.LUp())
				{
					vDir = normalize(vDir + V2d(0, -extraBUp));
				}
				else if (currInput.LDown())
				{
					vDir = normalize(vDir + V2d(0, extraBDown));
				}
				velocity = vDir * velStrength;
			}

			if (boostNow)
			{
				velocity += vDir * bounceBoostSpeed / (double)slowMultiple;


				/*if( currInput.LUp() )
				{
				velocity += V2d( 0, -1 ) * extraBUp;
				}
				else if( currInput.LDown() )
				{
				velocity += V2d( 0, 1 ) * extraBDown;
				}*/

				boostBounce = false;
			}
			else if (boostBounce)
			{
				boostBounce = false;
			}
		}

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
			edgeQuantity = bounceQuant;
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

			/*if( currInput.LLeft() || currInput.LRight() || currInput.LDown() || currInput.LUp() )
			{
			groundSpeed = dot( testVel, alongVel );
			}
			else
			{
			if( gNorm.y > -steepThresh )
			{
			groundSpeed = dot( testVel, alongVel );
			}
			else
			{
			groundSpeed = 0;
			}
			}*/

			//normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
			//cout << "setting groundSpeed: " << groundSpeed << endl;
			//V2d gNorm = ground->Normal();//minContact.normal;//ground->Normal();
			currNormal = ground->Normal();

			//if( gNorm.y <= -steepThresh )
			{
				RechargeAirOptions();
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
	if (!boostBounce && currInput.A && !prevInput.A)
	{
		//ActivateSound( soundBuffers[S_BOUNCEJUMP] );
		boostBounce = true;


		V2d bouncePos = bounceEdge->GetPosition(bounceQuant);
		V2d bn = bounceEdge->Normal();
		double angle = atan2(bn.x, -bn.y);
		bouncePos += bn * 80.0;
		ActivateEffect(EffectLayer::IN_FRONT, ts_bounceBoost, bouncePos, false, angle, 30, 1, facingRight);
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

	if (abs(bn.x) >= wallThresh)
	{
		if (bn.x > 0)
		{
			sprite->setOrigin(110, sprite->getLocalBounds().height / 2);
		}
		else
		{
			sprite->setOrigin(sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
		}
	}
	else if (bn.y <= 0 && bn.y > -steepThresh)
	{
		if (bounceFacingRight)
		{
			sprite->setOrigin(110, sprite->getLocalBounds().height / 2);
			//sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
		}
		else
		{
			//sprite->setOrigin( , sprite->getLocalBounds().height / 2);
			sprite->setOrigin(sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
		}
	}
	else if (bn.y >= 0 && -bn.y > -steepThresh)
	{
		if (bounceFacingRight)//bounceFrame == 4 )
		{
			sprite->setOrigin(110, sprite->getLocalBounds().height / 2);
			//sprite->setOrigin( sprite->getLocalBounds().width / 2, 0);
			//sprite->setOrigin( 0, sprite->getLocalBounds().height / 2);
		}
		else
		{
			sprite->setOrigin(sprite->getLocalBounds().width - 110, sprite->getLocalBounds().height / 2);
			//sprite->setOrigin( sprite->getLocalBounds().width, sprite->getLocalBounds().height / 2);
		}
	}
	else if (bn.y < 0)
	{
		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height - 120);
	}
	else if (bn.y > 0)
	{
		//cout << "this one" << endl;
		sprite->setOrigin(sprite->getLocalBounds().width / 2, 80);
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
	return GetActionTileset("bounce_224x224.png");
}