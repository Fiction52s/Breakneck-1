#include "Actor.h"

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
			SetActionExpr(BRAKE);
			//SetActionExpr(STAND);
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
				SetActionExpr(RUN);
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
				SetActionExpr(RUN);
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
			SetActionExpr(RUN);
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
		sprite->setPosition(pp.x + offsetX, pp.y);
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
		sprite->setPosition(pp.x, pp.y);
		along = normalize(ground->v1 - ground->v0);
	}

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
		ActivateSound(S_SPRINT_STEP1);
	}
	else if (frame == 6 * 4 && slowCounter == 1)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_sprint,
			pp + gn * 48.0 + along * xExtraStart, false, angle, 10, 2, facingRight);
		ActivateSound(S_SPRINT_STEP2);
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
}