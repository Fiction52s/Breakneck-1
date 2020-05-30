#include "Actor.h"
#include <assert.h>

using namespace sf;
using namespace std;

void Actor::GRAVREVERSE_Start()
{
}

void Actor::GRAVREVERSE_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::GRAVREVERSE_Change()
{
	if (hasPowerGrindBall && currInput.Y)//&& !prevInput.Y )
	{
		//only allow buffered reverse grind ball if you have gravity reverse. might remove it entirely later.
		if (!reversed || (hasPowerGravReverse && reversed))
		{
			groundSpeed = storedReverseSpeed;
			SetActionGrind();
			return;
		}
	}

	//groundSpeed = 0;
	if (reversed)
	{
		if (-currNormal.y > -steepThresh && approxEquals(abs(offsetX), b.rw))
		{

			if (groundSpeed < 0 && currNormal.x > 0 || groundSpeed > 0 && currNormal.x < 0)
			{
				if (groundSpeed > 0)
					facingRight = true;
				else
					facingRight = false;

				SetAction(STEEPCLIMB);

				frame = 0;
				return;
			}
			else
			{
				if (groundSpeed > 0)
					facingRight = true;
				else
					facingRight = false;
				SetAction(STEEPSLIDE);
				frame = 0;
				return;
			}

		}
		else
		{

			if ((currInput.B && !(reversed && (!currInput.LLeft() && !currInput.LRight()))) || !canStandUp)
			{
				//cout << "storedreversesddddpeed: " << storedReverseSpeed << endl;
				groundSpeed = storedReverseSpeed;
				//action = DASH;
				//frame = 0;
				SetActionExpr(DASH);

				if (currInput.LLeft())
					facingRight = false;
				else if (currInput.LRight())
					facingRight = true;
			}
			else if (currInput.LLeft() || currInput.LRight())
			{
				//cout << "storedreversespeed: " << storedReverseSpeed << endl;
				groundSpeed = storedReverseSpeed;
				SetActionExpr(RUN);
				frame = 0;
			}
			else if (TryGroundAttack())
			{
				return;
			}
			else if (!currInput.LDown())
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
				/*SetActionExpr( STAND );
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

	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
}