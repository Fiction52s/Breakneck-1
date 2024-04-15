#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::GRAVREVERSE_Start()
{
	currBBoostCounter = 0;
	assert(reversed);
}

void Actor::GRAVREVERSE_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::GRAVREVERSE_Change()
{
	if (CheckSetToAerialFromNormalWater()) return;


	if (CanBufferGrind())
	{
		//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
		//if (!reversed || (HasUpgrade(POWER_GRAV) && reversed))
		{
			groundSpeed = -storedReverseSpeed;
			SetActionGrind();
			return;
		}
	}

	

	//groundSpeed = 0;
	if (reversed)
	{
		V2d norm = GetGroundedNormal();
		if (-norm.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{

			if (groundSpeed < 0 && norm.x > 0 || groundSpeed > 0 && norm.x < 0)
			{
				SetAction(STEEPCLIMB);

				frame = 0;
				return;
			}
			else
			{
				SetAction(STEEPSLIDE);
				frame = 0;
				return;
			}

		}
		else
		{

			if ((DashButtonHeld() && !(reversed && (!currInput.LLeft() && !currInput.LRight()))) || !canStandUp)
			{
				//cout << "storedreversesddddpeed: " << storedReverseSpeed << endl;
				groundSpeed = -storedReverseSpeed;
				//action = DASH;
				//frame = 0;
				SetAction(DASH);

				if (currInput.LLeft())
					facingRight = false;
				else if (currInput.LRight())
					facingRight = true;
			}
			else if (currInput.LLeft() || currInput.LRight())
			{
				//cout << "storedreversespeed: " << storedReverseSpeed << endl;
				groundSpeed = -storedReverseSpeed;
				SetAction(RUN);
				frame = 0;

				/*if (currInput.LLeft())
					facingRight = false;
				else if (currInput.LRight())
					facingRight = true;*/
			}
			else if (TryGroundAttack())
			{
				return;
			}
			else if (!HoldingRelativeDown())
			{
				SetAction(STAND);
				frame = 0;
				/*bool okay = false;
				if( frame > 0 && !prevInput.LUp() )
				{
				action = SLIDE;
				frame = 0;
				}*/
			}
			else
			{
				/*SetAction( STAND );
				frame = 0;*/
			}
		}
	}
	else
	{
		assert(0 && "should be reversed here");
	}


	SetCurrHitboxes(shockwaveHitboxes, frame);
}

void Actor::GRAVREVERSE_Update()
{
}

void Actor::GRAVREVERSE_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

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

void Actor::GRAVREVERSE_TransitionToAction(int a)
{

}

void Actor::GRAVREVERSE_TimeIndFrameInc()
{

}

void Actor::GRAVREVERSE_TimeDepFrameInc()
{

}

int Actor::GRAVREVERSE_GetActionLength()
{
	return 20;
}

const char * Actor::GRAVREVERSE_GetTilesetName()
{
	return "grav_64x64.png";
}