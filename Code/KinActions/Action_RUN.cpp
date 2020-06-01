#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::RUN_Start()
{
}

void Actor::RUN_End()
{
	frame = 0;
}

void Actor::RUN_Change()
{
	BasicGroundAction(currNormal);
}

void Actor::RUN_Update()
{
	RunMovement();
}

void Actor::RUN_UpdateSprite()
{
	V2d pp = ground->GetPosition(edgeQuantity);

	double angle = GroundedAngle();


	//V2d along = normalize( ground->v1 - ground->v0 );
	SetSpriteTexture(RUN);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	int f = (frame / 2) % 10;
	SetSpriteTile(f, r);

	//assert(ground != NULL);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
	sprite->setRotation(angle / PI * 180);

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

	bool fr = facingRight;
	if (reversed)
		fr = !fr;

	double xExtraStartRun = -48.0;//0.0;//5.0
	if (!fr)
		xExtraStartRun = -xExtraStartRun;

	//this seems pretty odd. need a thing for its first repetition
	//so i dont need to check the controller here
	if (frame == 0 && slowCounter == 1 && (
		(currInput.LLeft() && !prevInput.LLeft())
		|| (currInput.LRight() && !prevInput.LRight())))
	{
		switch (speedLevel)
		{
		case 0:
			ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart[0],
				pp + gn * 40.0 + along * xExtraStartRun, false, angle, 6, 3, fr);
			break;
		case 1:
			ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart[1],
				pp + gn * 40.0 + along * xExtraStartRun, false, angle, 6, 3, fr);
			break;
		case 2:
			ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_runStart[2],
				pp + gn * 40.0 + along * xExtraStartRun, false, angle, 6, 3, fr);
			break;
		}
	}

	double xExtraStart = -48.0;
	if (!facingRight)
		xExtraStart = -xExtraStart;
	if (reversed)
		xExtraStart = -xExtraStart;


	if (frame == 3 * 4 && slowCounter == 1)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_run,
			pp + gn * 48.0 + along * xExtraStart, false, angle, 8, 3, fr);
		ActivateSound(S_RUN_STEP1);
	}
	else if (frame == 8 * 4 && slowCounter == 1)
	{
		ActivateEffect(EffectLayer::BETWEEN_PLAYER_AND_ENEMIES, ts_fx_run,
			pp + gn * 48.0 + along * xExtraStart, false, angle, 8, 3, fr);
		ActivateSound(S_RUN_STEP2);
	}


	if (frame % 5 == 0 && abs(groundSpeed) > 0)
	{
		//ActivateEffect( ts_fx_bigRunRepeat, pp + gn * 56.0, false, angle, 24, 1, facingRight );
	}

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpRun->texture);

		SetSpriteTile(&scorpSprite, ts_scorpRun, f / 2, fr);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 20);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::RUN_TransitionToAction(int a)
{

}

void Actor::RUN_TimeIndFrameInc()
{

}

void Actor::RUN_TimeDepFrameInc()
{

}

int Actor::RUN_GetActionLength()
{
	return 1;
}

Tileset * Actor::RUN_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}