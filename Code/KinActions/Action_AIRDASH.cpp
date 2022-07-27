#include "Actor.h"
#include "Wire.h"
#include "Enemy_Booster.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

void Actor::AIRDASH_Start()
{
	framesExtendingAirdash = 0;
	//BounceFlameOff();
	airDashStall = false;
	currAirdashBoostCounter = 0;
	holdJump = false;
	holdDouble = false;

	//special unlimited airdash
	if (inBubble && !hasAirDash)
	{
		frame = GetActionLength(AIRDASH) - 1;
	}
	else
	{
		frame = 0;
	}
}

void Actor::AIRDASH_End()
{
	if (CheckExtendedAirdash())//|| rightWire->state == Wire::PULLING )
	{
		//5 is here to give you extra frames to airdash
		frame = GetActionLength(AIRDASH) - 1;
		++framesExtendingAirdash;
		airDashStall = true;
	}
	else
	{
		TryAirdashBoost();


		SetAction(JUMP);
		frame = 1;
		holdJump = false;
	}
}

void Actor::AIRDASH_Change()
{
	if (!BasicAirAction())
	{
		if (!DashButtonHeld())
		{
			SetAction(JUMP);
			frame = 1;
			holdJump = false;

			if (rightWire->state == Wire::PULLING || leftWire->state == Wire::PULLING)
			{
			}
			else
			{
				//velocity = V2d( 0, 0 );
			}
		}
	}
}

void Actor::AIRDASH_Update()
{
	double aSpeed = GetAirDashSpeed();


	if (IsSingleWirePulling())
	{
		if (frame == 0)
		{
			hasAirDash = false;
			startAirDashVel = velocity;//V2d( velocity.x, 0 );//velocity;//
		}
	}
	else
	{
		bool isDoubleWiring = IsDoubleWirePulling();

		V2d startVel = velocity;
		//some old code in here for an alternate airdash when double wiring. ignore for now
		if (frame == 0)
		{
			dWireAirDash = V2d(0, 0);
			dWireAirDashOld = V2d(0, 0);

			hasAirDash = false;

			if (hasFairAirDashBoost)
			{
				if (velocity.x > 0)
				{
					velocity.x += fairAirDashBoostQuant;
				}
				else if (velocity.x < 0)
				{
					velocity.x -= fairAirDashBoostQuant;
				}
			}

			
			startAirDashVel = V2d(velocity.x, 0);//velocity;//
			if ((velocity.y > 0 && currInput.LDown()) || (velocity.y < 0 && currInput.LUp()))
			{
				if (abs(velocity.y) > aSpeed)
				{
					if (velocity.y < 0)
					{
						extraAirDashY = velocity.y + aSpeed;// / 2;
					}
					else
					{
						extraAirDashY = velocity.y - aSpeed;// / 2;
					}
				}
				else
				{
					extraAirDashY = velocity.y;//0;
				}

				//if (extraAirDashY > 0)
				//{
				//	extraAirDashY = .1;
				//	//extraAirDashY = min( extraAirDashY, 5.0 );
				//	//extraAirDashY *= 1.8;
				//}
			}
			else
			{
				extraAirDashY = 0;
			}
		}
		//V2d oldvel = velocity;

		dWireAirDash = V2d(0, 0);
		if (false)//isDoubleWiring )
		{
			if (currInput.LUp())
			{
				dWireAirDash += V2d(0, -aSpeed);
			}
			if (currInput.LLeft())
			{
				dWireAirDash += V2d(-aSpeed, 0);
			}
			if (currInput.LRight())
			{
				dWireAirDash += V2d(aSpeed, 0);
			}
			if (currInput.LDown())
			{
				dWireAirDash += V2d(0, aSpeed);
			}
		}
		else
		{
			velocity = V2d(0, 0);//startAirDashVel;

			double keepHorizontalLimit = 30;
			double removeSpeedFactor = .5;

			if (currInput.LUp())
			{
				if (!(currInput.LLeft() || currInput.LRight()) && abs(startAirDashVel.x) >= keepHorizontalLimit)
				{
					velocity.x = startAirDashVel.x * removeSpeedFactor;
					//cout << "velocity.x: " << velocity.x << endl;
				}

				if (extraAirDashY > 0)
					extraAirDashY = 0;

				double mod = 1.0;

				if (extraGravityModifier < 1.0)
				{
					mod = extraGravityModifier * 1.5;
					mod = min(1.0, mod);
					mod = max(.5, mod);
				}
				else
				{
					mod = extraGravityModifier * .75;
					mod = max(1.0, mod);
					mod = min(2.0, mod);
				}

				if (extraGravityModifier == 0) //0 gravity water
				{
					velocity.y = min(-aSpeed, startVel.y);
				}
				else
				{
					velocity.y = -aSpeed / mod + extraAirDashY;

					if (extraAirDashY < 0)
					{
						extraAirDashY = AddAerialGravity(V2d(0, extraAirDashY)).y;
						//extraAirDashY += gravity / slowMultiple;
						if (extraAirDashY > 0)
							extraAirDashY = 0;
					}
				}
				//extragravitymodifier must not be 0
				

				
			}
			else if (currInput.LDown())
			{
				if (!(currInput.LLeft() || currInput.LRight()) && abs(startAirDashVel.x) >= keepHorizontalLimit)
				{
					velocity.x = startAirDashVel.x * removeSpeedFactor;
					//cout << "velocity.x: " << velocity.x << endl;
				}


				if (extraGravityModifier == 0)
				{
					velocity.y = max(aSpeed, startVel.y);
				}
				else
				{
					if (extraAirDashY < 0)
						extraAirDashY = 0;

					velocity.y = aSpeed + extraAirDashY;

					if (extraAirDashY > 0)
					{
						extraAirDashY = AddAerialGravity(V2d(0, extraAirDashY)).y;
						//extraAirDashY += gravity / slowMultiple;
					}
				}

				


			}
			else
			{
				extraAirDashY = 0;
			}


			if (currInput.LLeft())
			{
				if (startAirDashVel.x > 0)
				{
					startAirDashVel.x = 0;
					velocity.x = -aSpeed;
				}
				else
				{
					velocity.x = min(startAirDashVel.x, -aSpeed);
				}
				facingRight = false;

			}
			else if (currInput.LRight())
			{
				if (startAirDashVel.x < 0)
				{
					startAirDashVel.x = 0;
					velocity.x = aSpeed;
				}
				else
				{
					velocity.x = max(startAirDashVel.x, aSpeed);
				}
				facingRight = true;
			}

			if (velocity.x == 0 && velocity.y == 0)
			{
				/*if( isDoubleWiring )
				{
				velocity = oldvel;
				}
				else*/
				{
					startAirDashVel = V2d(0, 0);
					extraAirDashY = 0;
					velocity = AddAerialGravity(velocity);
				}

			}

		}

		velocity -= dWireAirDashOld;
		velocity += dWireAirDash;

		dWireAirDashOld = dWireAirDash;
	}

	if (!simulationMode)
	{
		if (currBooster != NULL && oldBooster == NULL && currBooster->Boost())
		{
			SetBoostVelocity();

			startAirDashVel.x = velocity.x;
			extraAirDashY = velocity.y;
			if (extraAirDashY > aSpeed)
			{
				extraAirDashY = extraAirDashY - aSpeed;
			}
			else if (extraAirDashY < -aSpeed)
			{
				extraAirDashY = extraAirDashY + aSpeed;
			}
		}
	}

	//cout << "velocity.x: " << velocity.x << endl;
}

void Actor::AIRDASH_UpdateSprite()
{
	SetSpriteTexture(action);

	int f = 0;
	if (currInput.LUp())
	{
		if (currInput.LLeft() || currInput.LRight())
		{
			f = 2;
		}
		else
		{
			f = 1;
		}
	}
	else if (currInput.LDown())
	{
		if (currInput.LLeft() || currInput.LRight())
		{
			f = 4;
		}
		else
		{
			f = 5;
		}
	}
	else
	{
		if (currInput.LLeft() || currInput.LRight())
		{
			f = 3;
		}
		else
		{
			f = 0;
		}
	}

	SetSpriteTile(f, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	//spawn effects
	bool horizontal = abs(velocity.y) < 7; //for hover
	bool stopped = horizontal && velocity.x == 0;
	if (frame % 1 == 0 && !stopped)
	{
		//ActivateEffect( ts_fx_airdashSmall, V2d( position.x, position.y + 0 ), false, 0, 12, 4, facingRight );
	}

	if ((frame + framesExtendingAirdash) % 4 == 0 && slowCounter == 1)
	{
		if (stopped)
		{
			//cout << "frame: " << frame << endl;
			//if( frame % 4 == 1 )
			ActivateEffect(PLAYERFX_AIRDASH_HOVER, Vector2f(position.x, position.y + 70), 0, 12, 1, facingRight);
		}
		else if (horizontal)
		{
			//cout << "STUFF???" << endl;
			if (velocity.x > 0)
			{
				ActivateEffect(PLAYERFX_AIRDASH_UP, Vector2f(position.x - 64, position.y - 18), RadiansToDegrees(PI / 2.0), 15, 3, true);
			}
			else
			{
				ActivateEffect(PLAYERFX_AIRDASH_UP, Vector2f(position.x + 64, position.y - 18), RadiansToDegrees(-PI / 2.0), 15, 3, true);
			}
		}
		else if (velocity.x == 0 && velocity.y < 0)
		{
			ActivateEffect(PLAYERFX_AIRDASH_UP, Vector2f(position.x, position.y + 64), 0, 15, 3, facingRight);
		}
		else if (velocity.x == 0 && velocity.y > 0)
		{
			ActivateEffect(PLAYERFX_AIRDASH_UP, Vector2f(position.x, position.y), RadiansToDegrees( PI ), 15, 3, facingRight);
		}
		else if (velocity.x > 0 && velocity.y > 0)
		{
			V2d pos = V2d(position.x - 40, position.y - 60);
			ActivateEffect(PLAYERFX_AIRDASH_DIAGONAL, Vector2f(pos), RadiansToDegrees(PI), 15, 3, true);
		}
		else if (velocity.x < 0 && velocity.y > 0)
		{
			ActivateEffect(PLAYERFX_AIRDASH_DIAGONAL, Vector2f(position.x + 40, position.y - 60 ), RadiansToDegrees(PI), 15, 3, false);
		}
		else if (velocity.x < 0 && velocity.y < 0)
		{
			ActivateEffect(PLAYERFX_AIRDASH_DIAGONAL, Vector2f(position.x + 54, position.y + 60), 0, 15, 3, true);
		}
		else if (velocity.x > 0 && velocity.y < 0)
		{
			ActivateEffect(PLAYERFX_AIRDASH_DIAGONAL, Vector2f(position.x - 54, position.y + 60), 0, 15, 3, false);
		}

		//cout << "airdash fx" << endl;

	}

	if (scorpOn)
		SetAerialScorpSprite();
}


void Actor::AIRDASH_TransitionToAction(int a)
{

}

void Actor::AIRDASH_TimeIndFrameInc()
{

}

void Actor::AIRDASH_TimeDepFrameInc()
{

}

int Actor::AIRDASH_GetActionLength()
{
	return 33;
}

Tileset * Actor::AIRDASH_GetTileset()
{
	return GetActionTileset("airdash_80x80.png");
}
