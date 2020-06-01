#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DASH_Start()
{
	frame = 0;
	currBBoostCounter = 0;
}

void Actor::DASH_End()
{
	SetActionExpr(STAND);
	frame = 0;
}

void Actor::DASH_Change()
{
	CheckBounceFlame();

	if (TryGrind())return;



	if (currInput.A && !prevInput.A)
	{
		SetActionExpr(JUMPSQUAT);
		frame = 0;
		return;
	}

	if (TryGroundAttack())
	{
		return;
	}

	if (BasicSteepAction(currNormal))
		return;

	if (canStandUp)
	{
		if (!currInput.B)
		{
			if (currBBoostCounter >= 20)//maxBBoostCount - 15 )
			{
				double dashFactor = 1.85;//1.5;
				double bboostSpeed = GetDashSpeed() * dashFactor;

				if (bboostSpeed > abs(groundSpeed))
				{
					if (groundSpeed > 0)
					{
						groundSpeed = bboostSpeed;
					}
					else
					{
						groundSpeed = -bboostSpeed;
					}
				}



				/*double dashFactor = 3.0;
				double ag = abs(groundSpeed);

				if (ag > 30)
				{
				dashFactor = 3.0;
				}
				else
				{
				dashFactor = 2.0;
				}*/

				//double bboost = GetDashSpeed() / dashFactor;

				/*if (groundSpeed > 0)
				{
				groundSpeed += bboost;
				}
				else
				{
				groundSpeed -= bboost;
				}*/
				//currBBoostCounter = 0;
			}




			if (currInput.LLeft() || currInput.LRight())
			{
				SetActionExpr(RUN);
				frame = 0;
			}
			else if (currInput.LDown() || currInput.LUp())
			{
				SetAction(SLIDE);
				frame = 0;
			}
			else
			{
				SetAction(STAND);
				frame = 0;
			}
		}
	}
	else
	{
		//cout << "cant stand up" << endl;
		if (frame == actionLength[DASH] - 2)
			frame = 10;
	}
}

void Actor::DASH_Update()
{
	double dSpeed = GetDashSpeed();
	b.rh = dashHeight;
	b.offset.y = (normalHeight - dashHeight);
	if (reversed)
		b.offset.y = -b.offset.y;
	if (currInput.LLeft() && facingRight)
	{
		facingRight = false;
		groundSpeed = -dSpeed;
		frame = 0;
	}
	else if (currInput.LRight() && !facingRight)
	{
		facingRight = true;
		groundSpeed = dSpeed;
		frame = 0;
	}
	else if (!facingRight)
	{
		if (groundSpeed > -dSpeed)
			groundSpeed = -dSpeed;
	}
	else
	{
		if (groundSpeed < dSpeed)
			groundSpeed = dSpeed;
	}



	double sprFactor = 2.0; //dash must be slower accel on slopes
	if (currInput.LDown() && ((facingRight && currNormal.x > 0) || (!facingRight && currNormal.x < 0)))
	{
		double sprAccel = GetFullSprintAccel(true, currNormal) / sprFactor;
		if (facingRight)
		{
			groundSpeed += sprAccel / slowMultiple;
		}
		else
		{
			groundSpeed -= sprAccel / slowMultiple;
		}
	}
	else if (currInput.LUp() && ((facingRight && currNormal.x > 0) || (!facingRight && currNormal.x < 0)))
	{
		double sprAccel = GetFullSprintAccel(false, currNormal) / sprFactor;

		if (facingRight)
		{
			groundSpeed += sprAccel / slowMultiple;
		}
		else
		{
			groundSpeed -= sprAccel / slowMultiple;
		}
	}

	GroundExtraAccel();

	if (standNDashBoost)
	{
		if (groundSpeed > 0)
		{
			groundSpeed += standNDashBoostQuant;
		}
		else
		{
			groundSpeed -= standNDashBoostQuant;
		}
		standNDashBoost = false;
	}
}

void Actor::DASH_UpdateSprite()
{
	if (frame == 0)//&& currInput.B && !prevInput.B )
	{
		//dashStartSound.stop();
		//if( slowMultiple != 1)
		//	dashStartSound.setPitch( .2 );
		//else
		//	dashStartSound.setPitch( 1 );
		//cout << "playing dash sound" << endl;
		//dashStartSound.play();
		//dashStartSound.setLoop( true );
	}

	//if( slowMultiple != 1)
	//		dashStartSound.setPitch( .2 );
	//	else
	//		dashStartSound.setPitch( 1 );

	SetSpriteTexture(action);



	//3-8 is the cycle
	sf::IntRect ir;
	int checkFrame = -1;

	if (frame / 2 < 1)
	{
		checkFrame = frame / 2;
	}
	else if (frame < actionLength[DASH] - 1)
	{
		checkFrame = 1 + ((frame / 2 - 1) % 5);
	}
	else
	{
		checkFrame = 6;
	}

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(checkFrame, r);


	double angle = GroundedAngle();


	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	V2d along = normalize(ground->v1 - ground->v0);
	double xExtraRepeat = 64;
	double xExtraStart = 80;

	if ((facingRight && !reversed) || (!facingRight && reversed))
	{
		xExtraRepeat = -xExtraRepeat;
		xExtraStart = -xExtraStart;
	}

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		pp.x += offsetX;

	sprite->setPosition(pp.x, pp.y);

	bool fr = facingRight;
	if (reversed)
		fr = !fr;
	if (frame == 0 && currInput.B && !prevInput.B)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashStart,
			pp + currNormal * 64.0 + along * xExtraStart, false, angle, 9, 3, fr);
		ActivateSound(S_DASH_START);
	}
	else if (frame % 5 == 0)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_dashRepeat,
			pp + currNormal * 32.0 + along * xExtraRepeat, false, angle, 12, 3, fr);
	}

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpDash->texture);

		SetSpriteTile(&scorpSprite, ts_scorpDash, checkFrame, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::DASH_TransitionToAction(int a)
{

}

void Actor::DASH_TimeIndFrameInc()
{

}

void Actor::DASH_TimeDepFrameInc()
{

}

int Actor::DASH_GetActionLength()
{
	return 1;
}

Tileset * Actor::DASH_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}