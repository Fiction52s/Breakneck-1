#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SLIDE_Start()
{
	repeatingSound = ActivateSound(S_SLIDE, true);
}

void Actor::SLIDE_End()
{
	frame = 0;
}

void Actor::SLIDE_Change()
{
	BasicGroundAction(currNormal);
}

void Actor::SLIDE_Update()
{
	double gAngle = GroundedAngle();
	//double fac = gravity * 2.0 / 3;
	if (gAngle != 0 && gAngle != PI)
	{
		if (reversed)
		{
			double accel = dot(V2d(0, slideGravFactor * GetGravity()), normalize(ground->v1 - ground->v0)) / slowMultiple;
			groundSpeed += accel;

		}
		else
		{
			double accel = dot(V2d(0, slideGravFactor * GetGravity()), normalize(ground->v1 - ground->v0)) / slowMultiple;
			groundSpeed += accel;
			//cout << "accel slide: \n" << accel;
		}
	}
}

void Actor::SLIDE_UpdateSprite()
{
	SetSpriteTexture(SLIDE);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	SetGroundedSpriteTransform();

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 15);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::SLIDE_TransitionToAction(int a)
{

}

void Actor::SLIDE_TimeIndFrameInc()
{

}

void Actor::SLIDE_TimeDepFrameInc()
{

}

int Actor::SLIDE_GetActionLength()
{
	return 1;
}

Tileset * Actor::SLIDE_GetTileset()
{
	return GetActionTileset("slide_64x64.png");
}