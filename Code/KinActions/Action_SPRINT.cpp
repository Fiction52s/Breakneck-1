#include "Actor.h"
#include "Session.h"
#include "VisualEffects.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::SPRINT_Start()
{
}

void Actor::SPRINT_End()
{
	frame = 0;
}

void Actor::SPRINT_Change()
{
	if (BasicGroundAction(currNormal))
		return;

	if (!(currInput.LLeft() || currInput.LRight()))
	{
		if (currInput.LDown() || currInput.LUp())
		{
			SetAction(SLIDE);
			frame = 0;
			return;
		}
		else
		{
			SetAction(BRAKE);
			//SetAction(STAND);
			frame = 0;
			return;
		}
	}
	else
	{
		if (facingRight && currInput.LLeft())
		{

			if ((currInput.LDown() && currNormal.x < 0) 
				|| (currInput.LUp() && currNormal.x > 0))
			{
				frame = 0;
			}
			else
			{
				SetAction(RUN);
			}

			groundSpeed = 0;
			facingRight = false;
			frame = 0;
			return;
		}
		else if (!facingRight && currInput.LRight())
		{
			if ((currInput.LDown() && currNormal.x > 0) 
				|| (currInput.LUp() && currNormal.x < 0))
			{
				frame = 0;
			}
			else
			{
				SetAction(RUN);
			}

			groundSpeed = 0;
			facingRight = true;
			frame = 0;
			return;
		}
		else if (!((currInput.LDown() && ((currNormal.x > 0 && facingRight) 
			|| (currNormal.x < 0 && !facingRight)))
			|| (currInput.LUp() && ((currNormal.x < 0 && facingRight) 
				|| (currNormal.x > 0 && !facingRight)))))
		{
			SetAction(RUN);
			frame = frame / 4;
			if (frame < 3)
			{
				frame = frame + 1;
			}
			else if (frame == 3 || frame == 4)
			{
				frame = 7;
			}
			else if (frame == 5 || frame == 6)
			{
				frame = 8;
			}
			else if (frame == 7)
			{
				frame = 2;
			}
			frame = frame * 4;
			return;
		}

	}
}

void Actor::SPRINT_Update()
{
	if (b.rh > sprintHeight || canStandUp)
	{
		b.rh = sprintHeight;
		b.offset.y = (normalHeight - sprintHeight);

		if (reversed)
			b.offset.y = -b.offset.y;
	}

	if (currInput.LLeft())
		facingRight = false;
	else if (currInput.LRight())
		facingRight = true;

	double accel = 0;
	if (!facingRight)//currInput.LLeft() )
	{

		if (groundSpeed > 0)
		{
			groundSpeed = 0;
		}
		else
		{
			if (groundSpeed > -maxRunInit)
			{
				groundSpeed -= runAccelInit * 2 / slowMultiple;
				if (groundSpeed < -maxRunInit)
					groundSpeed = -maxRunInit;
			}
			else
			{


				if (currNormal.x > 0)
				{
					//up a slope
					double sprAccel = GetFullSprintAccel(false, currNormal);

					//GroundExtraAccel();

					accel = sprAccel / slowMultiple;
					groundSpeed -= accel;

				}
				else
				{
					//GroundExtraAccel();

					double sprAccel = GetFullSprintAccel(true, currNormal);

					accel = sprAccel / slowMultiple;
					groundSpeed -= accel;


					//down a slope
				}
			}

		}
		facingRight = false;
	}
	//else if( currInput.LRight() )
	else
	{
		if (groundSpeed < 0)
			groundSpeed = 0;
		else
		{
			V2d gn = ground->Normal();
			if (groundSpeed < maxRunInit)
			{
				groundSpeed += runAccelInit * 2 / slowMultiple;
				if (groundSpeed > maxRunInit)
					groundSpeed = maxRunInit;
			}
			else
			{
				double minFactor = .2;
				double factor = abs(currNormal.x);
				factor = std::max(factor, minFactor);

				if (currNormal.x < 0)
				{
					//GroundExtraAccel();

					double sprAccel = GetFullSprintAccel(false, currNormal);

					accel = sprAccel / slowMultiple;
					groundSpeed += accel;
				}
				else
				{

					//GroundExtraAccel();

					double sprAccel = GetFullSprintAccel(true, currNormal);

					accel = sprAccel / slowMultiple;
					groundSpeed += accel;
					//down a slope
				}
			}
		}
		facingRight = true;
	}

	GroundExtraAccel();
}

void Actor::SPRINT_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	int tFrame = frame / 2;
	SetSpriteTile(tFrame, r);

	//assert(ground != NULL);

	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

	V2d pp = ground->GetPosition(edgeQuantity);

	V2d along;
	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
	{
		if (!reversed)
		{
			along = V2d(1, 0);
		}
		else
		{
			along = V2d(-1, 0);
		}
	}
	else
	{
		along = normalize(ground->v1 - ground->v0);
	}

	SetGroundedSpriteTransform();

	V2d gn(along.y, -along.x);


	double xExtraStart = -48.0;
	if (!facingRight)
		xExtraStart = -xExtraStart;
	if (reversed)
		xExtraStart = -xExtraStart;

	if (frame == 2 * 4 && slowCounter == 1)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_sprint,
			pp + gn * 48.0 + along * xExtraStart, false, angle, 10, 2, facingRight);
		ActivateSound(PlayerSounds::S_SPRINT_STEP1);
	}
	else if (frame == 6 * 4 && slowCounter == 1)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_sprint,
			pp + gn * 48.0 + along * xExtraStart, false, angle, 10, 2, facingRight);
		ActivateSound(PlayerSounds::S_SPRINT_STEP2);
	}

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSprint->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSprint, tFrame / 2, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 20);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}

	//if (!IsIntroAction(action) && sess->totalGameFrames % 10 == 0)
	//{
	//	RelEffectInstance params;
	//	//EffectInstance params;
	//	Transform tr = sf::Transform::Identity;

	//	int dist = 80;
	//	//params.SetParams(Vector2f(position.x, position.y - 100) , tr, 7, 1, 0);
	//	Vector2f randPos(rand() % dist - dist/2, rand() % dist - dist/2);

	//	params.SetParams(randPos, tr, 24, 1, 0, &spriteCenter);

	//	sprintSparkPool->ActivateEffect(&params);
	//}
}

void Actor::SPRINT_TransitionToAction(int a)
{

}

void Actor::SPRINT_TimeIndFrameInc()
{

}

void Actor::SPRINT_TimeDepFrameInc()
{

}

int Actor::SPRINT_GetActionLength()
{
	return 8 * 4;
}

Tileset * Actor::SPRINT_GetTileset()
{
	return GetActionTileset("sprint_80x48.png");
}