#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WAITFORSHIP_Start()
{
}

void Actor::WAITFORSHIP_End()
{
	frame = 0;
}

void Actor::WAITFORSHIP_Change()
{
}

void Actor::WAITFORSHIP_Update()
{
}

void Actor::WAITFORSHIP_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(0, r);

	if (ground != NULL)
	{
		double angle = GroundedAngle();

		sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

		V2d pp = ground->GetPosition(edgeQuantity);

		if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
			sprite->setPosition(pp.x + offsetX, pp.y);
		else
			sprite->setPosition(pp.x, pp.y);
		sprite->setRotation(angle / PI * 180);


		//cout << "angle: " << angle / PI * 180  << endl;
	}
}

void Actor::WAITFORSHIP_TransitionToAction(int a)
{

}

void Actor::WAITFORSHIP_TimeIndFrameInc()
{

}

void Actor::WAITFORSHIP_TimeDepFrameInc()
{

}

int Actor::WAITFORSHIP_GetActionLength()
{
	return 60 * 1;
}

Tileset * Actor::WAITFORSHIP_GetTileset()
{
	return GetActionTileset("shipjump_160x96.png");
}